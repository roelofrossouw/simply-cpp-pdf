#include "pdf.h"

#include "font_collection.h"

#include <CatalogInformation.h>
#include <DictionaryContext.h>
#include <DocumentContextExtenderAdapter.h>
#include <EncryptionOptions.h>
#include <InfoDictionary.h>
#include <ObjectsContext.h>
#include <PDFPage.h>
#include <InputStringStream.h>
#include <PDFFormXObject.h>
#include <PDFImageXObject.h>
#include <PDFUsedFont.h>
#include <PDFWriter.h>
#include <PDFTextString.h>
#include <PageContentContext.h>
#include <ResourcesDictionary.h>
#include <TrailerInformation.h>
#include <XObjectContentContext.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sc {
    namespace {
        // A height from the font's OS/2 table, in points; when the font doesn't give it (DejaVu, for
        // one), how far letter rises above the baseline.
        double glyph_height(PDFUsedFont *font, const std::pair<bool, FT_Short> &declared, const char letter, const double size) {
            if (declared.first && declared.second > 0) return declared.second * size / 1000; // thousandths of the size
            FT_Face face = *font->GetFreeTypeFont();
            if (!face || face->units_per_EM == 0 || FT_Load_Char(face, letter, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING) != 0) return 0;
            return static_cast<double>(face->glyph->metrics.horiBearingY) * size / face->units_per_EM;
        }
    }

    struct pdf::impl {
        struct page_state {
            PDFPage *page;
            PageContentContext *content;
            // PDFHummus attaches links to the next page it writes, so they wait here until then.
            std::vector<std::pair<std::string, rect> > links;
            // Links within the file, to a page and a height on it, written after every page is.
            struct jump {
                rect area;
                unsigned page;
                double top;
            };
            std::vector<jump> jumps;
            // The ExtGStates the page uses, by their names in its resources.
            std::map<ObjectIDType, std::string> alpha_names;
            // The images it uses, the same way.
            std::map<ObjectIDType, std::string> image_names;
        };

        // An image written into the file: a JPEG as an image XObject (a unit square), a PNG as the
        // form XObject PDFHummus makes of it (its size in pixels).
        struct written_image {
            sc::bitmap bitmap; // held, so its bytes' address isn't another's
            ObjectIDType id;
            bool form;
        };

        PDFWriter writer;
        font_collection fonts{writer};
        double width;
        double height;
        std::vector<page_state> pages;
        unsigned current = 0; // index into pages
        bool closed = false;
        // One graphics state per (fill, stroke) alpha pair, shared by all pages; only the IDs are
        // taken while drawing, the objects are written by close(), as nothing may be written while a
        // page's content is open.
        std::map<std::pair<double, double>, ObjectIDType> alpha_states;
        // Each bitmap written once, by its bytes (copies share them), and used from any page.
        std::map<const std::string *, written_image> images;
        struct bookmark {
            std::string title;
            unsigned page;
            double top;
            int level;
        };
        std::vector<bookmark> outline;
        std::map<std::string, std::string> info;

        // Puts the outline into the catalog when PDFHummus writes it.
        struct catalog_outline : DocumentContextExtenderAdapter {
            ObjectIDType root = 0;

            PDFHummus::EStatusCode OnCatalogWrite(CatalogInformation *, DictionaryContext *catalog, ObjectsContext *, PDFHummus::DocumentContext *) override {
                if (root == 0) return PDFHummus::eSuccess;
                catalog->WriteKey("Outlines");
                catalog->WriteObjectReferenceValue(root);
                catalog->WriteKey("PageMode");
                catalog->WriteNameValue("UseOutlines");
                return PDFHummus::eSuccess;
            }
        } outline_writer;

        impl(const std::filesystem::path &filename, const page_size size, const orientation turned, const std::optional<protection> &locked = {}) {
            const auto page = page_dimensions(size, turned);
            width = page.width();
            height = page.height();
            PDFCreationSettings settings(true, true);
            if (locked && (!locked->user_password.empty() || !locked->owner_password.empty())) {
                // The permission bits (PDF 32000, table 22): 3 print, 4 modify, 5 copy, 12 print well.
                long long rights = 0;
                if (locked->print) rights |= 1 << 2 | 1 << 11;
                if (locked->edit) rights |= 1 << 3;
                if (locked->copy) rights |= 1 << 4;
                const auto &owner = locked->owner_password.empty() ? locked->user_password : locked->owner_password;
                settings = PDFCreationSettings(true, true, EncryptionOptions(locked->user_password, rights, owner));
            }
            if (writer.StartPDF(filename.string(), ePDFVersion14, LogConfiguration::DefaultLogConfiguration(), settings) != PDFHummus::eSuccess)
                throw std::runtime_error("sc::pdf: can't create " + filename.string());
            writer.GetDocumentContext().AddDocumentContextExtender(&outline_writer);
            add_page();
        }

        // PDF counts y up from the bottom of the page; sc::pdf, like a screen, down from the top.
        [[nodiscard]] double y(const double page_y) const { return height - page_y; }

        void check_open() const {
            if (closed) throw std::logic_error("sc::pdf: drawing after close()");
        }

        PageContentContext &content() {
            check_open();
            return *pages[current].content;
        }

        // Makes what follows (up to the content's next Q) paint at these alphas; opaque needs nothing.
        void set_alpha(PageContentContext &content, const double fill_alpha, const double stroke_alpha) {
            if (fill_alpha >= 1 && stroke_alpha >= 1) return;
            auto state = alpha_states.find({fill_alpha, stroke_alpha});
            if (state == alpha_states.end())
                state = alpha_states.emplace(std::pair{fill_alpha, stroke_alpha},
                                             writer.GetObjectsContext().GetInDirectObjectsRegistry().AllocateNewObjectID()).first;
            auto &page = pages[current];
            auto name = page.alpha_names.find(state->second);
            if (name == page.alpha_names.end())
                name = page.alpha_names.emplace(state->second, page.page->GetResourcesDictionary().AddExtGStateMapping(state->second)).first;
            content.gs(name->second);
        }

        // The outline's items, each pointing at its parent, siblings and first and last children,
        // with the root (named in the catalog) over the outermost ones.
        template<typename Destination>
        void write_outline(const Destination &destination, const std::size_t page_count) {
            if (outline.empty()) return;
            auto &objects = writer.GetObjectsContext();
            auto &registry = objects.GetInDirectObjectsRegistry();
            const std::size_t count = outline.size();
            std::vector<ObjectIDType> ids(count);
            for (auto &id: ids) id = registry.AllocateNewObjectID();
            const ObjectIDType root = registry.AllocateNewObjectID();
            // Each item's parent: the nearest item before it at a shallower level, else the root.
            std::vector<std::optional<std::size_t> > parent(count);
            std::vector<std::size_t> open; // the items still taking children, outermost first
            for (std::size_t i = 0; i < count; ++i) {
                while (!open.empty() && outline[open.back()].level >= outline[i].level) open.pop_back();
                if (!open.empty()) parent[i] = open.back();
                open.push_back(i);
            }
            const auto children_of = [&](const std::optional<std::size_t> &of) {
                std::vector<std::size_t> out;
                for (std::size_t i = 0; i < count; ++i)
                    if (parent[i] == of) out.push_back(i);
                return out;
            };
            for (std::size_t i = 0; i < count; ++i) {
                const auto &item = outline[i];
                if (item.page == 0 || item.page > page_count) throw std::out_of_range("sc::pdf: a bookmark to page " + std::to_string(item.page) + ", which isn't there");
                const auto siblings = children_of(parent[i]);
                const auto at = std::ranges::find(siblings, i) - siblings.begin();
                const auto children = children_of(i);
                objects.StartNewIndirectObject(ids[i]);
                DictionaryContext *entry = objects.StartDictionary();
                PDFTextString title;
                title.FromUTF8(item.title);
                entry->WriteKey("Title");
                entry->WriteLiteralStringValue(title.ToString());
                entry->WriteKey("Parent");
                entry->WriteObjectReferenceValue(parent[i] ? ids[*parent[i]] : root);
                if (at > 0) {
                    entry->WriteKey("Prev");
                    entry->WriteObjectReferenceValue(ids[siblings[at - 1]]);
                }
                if (at + 1 < static_cast<std::ptrdiff_t>(siblings.size())) {
                    entry->WriteKey("Next");
                    entry->WriteObjectReferenceValue(ids[siblings[at + 1]]);
                }
                if (!children.empty()) {
                    entry->WriteKey("First");
                    entry->WriteObjectReferenceValue(ids[children.front()]);
                    entry->WriteKey("Last");
                    entry->WriteObjectReferenceValue(ids[children.back()]);
                    entry->WriteKey("Count");
                    entry->WriteIntegerValue(-static_cast<long long>(children.size())); // closed
                }
                destination(entry, item.page, item.top);
                objects.EndDictionary(entry);
                objects.EndIndirectObject();
            }
            const auto top_level = children_of(std::nullopt);
            objects.StartNewIndirectObject(root);
            DictionaryContext *outlines = objects.StartDictionary();
            outlines->WriteKey("Type");
            outlines->WriteNameValue("Outlines");
            outlines->WriteKey("First");
            outlines->WriteObjectReferenceValue(ids[top_level.front()]);
            outlines->WriteKey("Last");
            outlines->WriteObjectReferenceValue(ids[top_level.back()]);
            outlines->WriteKey("Count");
            outlines->WriteIntegerValue(static_cast<long long>(top_level.size()));
            objects.EndDictionary(outlines);
            objects.EndIndirectObject();
            outline_writer.root = root;
        }

        // The bitmap written into the file, the first time it's drawn. Throws std::runtime_error
        // when PDFHummus can't read it.
        const written_image &image(const sc::bitmap &picture) {
            if (const auto found = images.find(&picture.bytes()); found != images.end()) return found->second;
            pause(); // the image is written now, outside the page's content
            InputStringStream stream{picture.bytes()};
            written_image result{picture, 0, picture.type() == bitmap::format::png};
            if (result.form) {
                PDFFormXObject *form = writer.CreateFormXObjectFromPNGStream(&stream);
                if (!form) throw std::runtime_error("sc::pdf: can't read the PNG image");
                result.id = form->GetObjectID();
                delete form;
            } else {
                PDFImageXObject *image = writer.CreateImageXObjectFromJPGStream(&stream);
                if (!image) throw std::runtime_error("sc::pdf: can't read the JPEG image");
                result.id = image->GetImageObjectID();
                delete image;
            }
            return images.emplace(&picture.bytes(), std::move(result)).first->second;
        }

        // Fills (by the fill's rule) and/or strokes what path() writes; whichever is none is left out.
        template<typename Path>
        void paint(const sc::appearance &look, Path path) {
            const auto &stroke = look.stroke();
            const auto &fill = look.fill();
            auto &page = content();
            const bool stroked = stroke.visible(), filled = fill.visible();
            const bool even_odd = fill.rule() == fill_rule::even_odd;
            page.q();
            set_alpha(page, filled ? fill.colour().alpha() : 1, stroked ? stroke.colour().alpha() : 1);
            if (filled) page.rg(fill.colour().red(), fill.colour().green(), fill.colour().blue());
            if (stroked) {
                page.w(stroke.width());
                page.RG(stroke.colour().red(), stroke.colour().green(), stroke.colour().blue());
            }
            path(page);
            if (stroked && filled) even_odd ? page.BStar() : page.B();
            else if (filled) even_odd ? page.fStar() : page.f();
            else page.S();
            page.Q();
        }

        void add_page() {
            pause();
            auto *page = new PDFPage();
            page->SetMediaBox(PDFRectangle(0, 0, width, height));
            pages.push_back({page, writer.StartPageContentContext(page), {}});
            current = static_cast<unsigned>(pages.size() - 1);
        }

        // PDFHummus needs the open page content paused before it writes another object (another
        // page's content, an image or form). Using the context again carries on where it stopped.
        void pause() {
            if (!pages.empty()) writer.PausePageContentContext(pages[current].content);
        }

        void close() {
            if (closed) return;
            closed = true;
            // Writing a page writes what it uses (fonts, images) too: no page's content stream may be
            // open then, or those objects land inside it.
            pause();
            bool written = true;
            auto &objects = writer.GetObjectsContext();
            auto &registry = objects.GetInDirectObjectsRegistry();
            // Links within the file name pages written later, so their objects come after the
            // pages; each page is told of its own now.
            struct pending_jump {
                ObjectIDType id;
                impl::page_state::jump jump;
            };
            std::vector<pending_jump> jumps;
            std::vector<ObjectIDType> page_ids;
            for (auto &state: pages) {
                written = writer.EndPageContentContext(state.content) == PDFHummus::eSuccess && written;
                for (const auto &[url, area]: state.links)
                    writer.AttachURLLinktoCurrentPage(url, PDFRectangle(area.left(), y(area.bottom()), area.right(), y(area.top())));
                for (const auto &jump: state.jumps) {
                    const ObjectIDType id = registry.AllocateNewObjectID();
                    writer.GetDocumentContext().RegisterAnnotationReferenceForNextPageWrite(id);
                    jumps.push_back({id, jump});
                }
                const auto [status, id] = writer.WritePageAndReturnPageID(state.page);
                written = status == PDFHummus::eSuccess && written;
                page_ids.push_back(id);
                delete state.page;
            }
            pages.clear();
            // [page 0 R /XYZ left top null]: the page, from that height, at the reader's zoom.
            const auto destination = [&](DictionaryContext *dictionary, const unsigned page, const double top) {
                dictionary->WriteKey("Dest");
                objects.StartArray();
                objects.WriteIndirectObjectReference(page_ids.at(page - 1));
                objects.WriteName("XYZ");
                objects.WriteInteger(0);
                objects.WriteDouble(y(top));
                objects.WriteKeyword("null");
                objects.EndArray(eTokenSeparatorEndLine);
            };
            for (const auto &[id, jump]: jumps) {
                if (jump.page == 0 || jump.page > page_ids.size()) throw std::out_of_range("sc::pdf: a link to page " + std::to_string(jump.page) + ", which isn't there");
                objects.StartNewIndirectObject(id);
                DictionaryContext *link = objects.StartDictionary();
                link->WriteKey("Type");
                link->WriteNameValue("Annot");
                link->WriteKey("Subtype");
                link->WriteNameValue("Link");
                link->WriteKey("Rect");
                link->WriteRectangleValue(PDFRectangle(jump.area.left(), y(jump.area.bottom()), jump.area.right(), y(jump.area.top())));
                link->WriteKey("Border");
                objects.StartArray();
                for (int i = 0; i < 3; ++i) objects.WriteInteger(0);
                objects.EndArray(eTokenSeparatorEndLine);
                destination(link, jump.page, jump.top);
                objects.EndDictionary(link);
                objects.EndIndirectObject();
            }
            write_outline(destination, page_ids.size());
            for (const auto &[key, value]: info) {
                auto &dictionary = writer.GetDocumentContext().GetTrailerInformation().GetInfo();
                PDFTextString text;
                text.FromUTF8(value);
                if (key == "Title") dictionary.Title = text;
                else if (key == "Author") dictionary.Author = text;
                else if (key == "Subject") dictionary.Subject = text;
                else if (key == "Keywords") dictionary.Keywords = text;
                else if (key == "Creator") dictionary.Creator = text;
                else if (key == "Producer") dictionary.Producer = text;
                else dictionary.AddAdditionalInfoEntry(key, text);
            }
            for (const auto &[alphas, id]: alpha_states) {
                objects.StartNewIndirectObject(id);
                DictionaryContext *state = objects.StartDictionary();
                state->WriteKey("Type");
                state->WriteNameValue("ExtGState");
                state->WriteKey("ca"); // fills and text
                state->WriteDoubleValue(alphas.first);
                state->WriteKey("CA"); // strokes
                state->WriteDoubleValue(alphas.second);
                objects.EndDictionary(state);
                objects.EndIndirectObject();
            }
            if (writer.EndPDF() != PDFHummus::eSuccess || !written) throw std::runtime_error("sc::pdf: writing the file failed");
        }
    };

    pdf::pdf(const std::filesystem::path &filename, const page_size size, const orientation turned)
        : impl_(std::make_unique<impl>(filename, size, turned)) {
    }

    pdf::pdf(const std::filesystem::path &filename, const page_size size, const orientation turned, const protection &locked)
        : impl_(std::make_unique<impl>(filename, size, turned, locked)) {
    }

    pdf::~pdf() {
        if (!impl_) return;
        try {
            impl_->close();
        } catch (const std::exception &error) {
            std::cerr << error.what() << '\n';
        }
    }

    pdf::pdf(pdf &&) noexcept = default;

    pdf &pdf::operator=(pdf &&other) noexcept {
        if (this != &other) {
            if (impl_) {
                try {
                    impl_->close();
                } catch (const std::exception &error) {
                    std::cerr << error.what() << '\n';
                }
            }
            impl_ = std::move(other.impl_);
        }
        return *this;
    }

    void pdf::close() {
        impl_->close();
    }

    void pdf::new_page() {
        impl_->check_open();
        impl_->add_page();
    }

    void pdf::set_page(const unsigned page) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::set_page: pages count from 1");
        while (impl_->pages.size() < page) impl_->add_page();
        if (impl_->current == page - 1) return;
        impl_->pause();
        impl_->current = page - 1;
    }

    unsigned pdf::page() const {
        return impl_->current + 1;
    }

    unsigned pdf::page_count() const {
        return static_cast<unsigned>(impl_->pages.size());
    }

    double pdf::page_width() const {
        return impl_->width;
    }

    double pdf::page_height() const {
        return impl_->height;
    }

    double pdf::measure_width(const text &sample) {
        return impl_->fonts.get(sample.font)->CalculateTextAdvance(sample.content, sample.font.size);
    }

    double pdf::measure_x_height(const font &font) {
        PDFUsedFont *used = impl_->fonts.get(font);
        return glyph_height(used, used->GetFreeTypeFont()->GetxHeight(), 'x', font.size);
    }

    double pdf::measure_cap_height(const font &font) {
        PDFUsedFont *used = impl_->fonts.get(font);
        return glyph_height(used, used->GetFreeTypeFont()->GetCapHeight(), 'H', font.size);
    }

    void pdf::paint_text(const text &sample, const sc::fill &fill) {
        auto &content = impl_->content();
        PDFUsedFont *used = impl_->fonts.get(sample.font);
        content.q();
        impl_->set_alpha(content, fill.colour().alpha(), 1);
        content.BT();
        content.rg(fill.colour().red(), fill.colour().green(), fill.colour().blue());
        content.Tf(used, sample.font.size);
        content.Td(sample.position.x(), impl_->y(sample.position.y()));
        content.Tj(sample.content);
        content.ET();
        content.Q();
    }

    void pdf::paint_line(const line &shape, const sc::stroke &stroke) {
        impl_->paint(stroke, [&](PageContentContext &content) {
            content.m(shape.from().x(), impl_->y(shape.from().y()));
            content.l(shape.to().x(), impl_->y(shape.to().y()));
        });
    }

    void pdf::paint_rounded_rect(const rounded_rect &shape, const sc::appearance &look) {
        const rect box = shape.rect();
        const double radius = shape.radius();
        const double left = box.left(), right = box.right(), bottom = impl_->y(box.bottom()), top = impl_->y(box.top());
        impl_->paint(look, [&](PageContentContext &content) {
            const double control = radius * (1 - quarter_circle); // from the corner
            content.m(left + radius, bottom);
            content.l(right - radius, bottom);
            content.c(right - control, bottom, right, bottom + control, right, bottom + radius);
            content.l(right, top - radius);
            content.c(right, top - control, right - control, top, right - radius, top);
            content.l(left + radius, top);
            content.c(left + control, top, left, top - control, left, top - radius);
            content.l(left, bottom + radius);
            content.c(left, bottom + control, left + control, bottom, left + radius, bottom);
            content.h();
        });
    }

    void pdf::paint_circle(const circle &shape, const sc::appearance &look) {
        const double x = shape.center().x(), y = impl_->y(shape.center().y()), r = shape.radius();
        const double k = r * quarter_circle; // control point distance from the end points (sc-core's path.h)
        impl_->paint(look, [&](PageContentContext &content) {
            content.m(x + r, y);
            content.c(x + r, y + k, x + k, y + r, x, y + r);
            content.c(x - k, y + r, x - r, y + k, x - r, y);
            content.c(x - r, y - k, x - k, y - r, x, y - r);
            content.c(x + k, y - r, x + r, y - k, x + r, y);
            content.h();
        });
    }

    void pdf::paint_polygon(const polygon &shape, const sc::appearance &look) {
        impl_->paint(look, [&](PageContentContext &content) {
            content.m(shape[0].x(), impl_->y(shape[0].y()));
            for (std::size_t i = 1; i < shape.size(); ++i) content.l(shape[i].x(), impl_->y(shape[i].y()));
            content.h();
        });
    }

    void pdf::draw_image(const std::filesystem::path &filename, const rect &pos) {
        auto &content = impl_->content();
        const auto file = filename.string();
        const auto [image_width, image_height] = impl_->writer.GetImageDimensions(file);
        if (image_width <= 0 || image_height <= 0) throw std::runtime_error("sc::pdf: can't read the image " + file);

        const double scale = std::min(pos.width() / image_width, pos.height() / image_height);
        AbstractContentContext::ImageOptions options;
        options.transformationMethod = AbstractContentContext::eMatrix;
        options.matrix[0] = options.matrix[3] = scale;
        const double x = pos.left() + (pos.width() - image_width * scale) / 2;
        const double y = impl_->y(pos.bottom()) + (pos.height() - image_height * scale) / 2;
        impl_->pause();
        if (content.DrawImage(x, y, file, options) != PDFHummus::eSuccess) throw std::runtime_error("sc::pdf: can't draw the image " + file);
    }

    void pdf::paint_bitmap(const bitmap &picture, const transform &placement) {
        impl_->check_open();
        const auto &written = impl_->image(picture);
        auto &state = impl_->pages[impl_->current];
        auto name = state.image_names.find(written.id);
        if (name == state.image_names.end()) {
            auto &resources = state.page->GetResourcesDictionary();
            name = state.image_names.emplace(written.id, written.form ? resources.AddFormXObjectMapping(written.id)
                                                                       : resources.AddImageXObjectMapping(written.id)).first;
        }
        auto &content = impl_->content();
        // The unit square, y up in PDF, through placement (y down from the top) onto the page.
        const auto &m = placement;
        content.q();
        content.cm(m.a(), -m.b(), -m.c(), m.d(), m.c() + m.e(), impl_->height - m.d() - m.f());
        if (written.form) content.cm(1 / picture.size().width(), 0, 0, 1 / picture.size().height(), 0, 0);
        content.Do(name->second);
        content.Q();
    }

    namespace {
        // Writes shape's steps into content, y flipped to PDF's.
        void trace(PageContentContext &content, const path &shape, const double page_height) {
            const auto y = [page_height](const point &p) { return page_height - p.y(); };
            for (const auto &step: shape.steps()) {
                switch (step.type) {
                    case path::kind::move: content.m(step.to.x(), y(step.to)); break;
                    case path::kind::line: content.l(step.to.x(), y(step.to)); break;
                    case path::kind::cubic:
                        content.c(step.first.x(), y(step.first), step.second.x(), y(step.second), step.to.x(), y(step.to));
                        break;
                    case path::kind::close: content.h(); break;
                }
            }
        }
    }

    void pdf::paint_path(const path &shape, const sc::appearance &look) {
        impl_->paint(look, [&](PageContentContext &content) { trace(content, shape, impl_->height); });
    }

    void pdf::push_clip(const path &shape) {
        auto &content = impl_->content();
        content.q();
        trace(content, shape, impl_->height);
        content.W(); // nonzero, as canvas::push_clip() clips
        content.n();
    }

    void pdf::pop_clip() {
        impl_->content().Q();
    }

    void pdf::add_link(const std::string &url, const rect &pos) {
        impl_->check_open();
        impl_->pages[impl_->current].links.emplace_back(url, pos);
    }

    void pdf::add_link(const unsigned page, const rect &pos, const double top) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::add_link: pages count from 1");
        impl_->pages[impl_->current].jumps.push_back({pos, page, top});
    }

    void pdf::add_outline(const std::string &title, const unsigned page, const double top, const int level) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::add_outline: pages count from 1");
        impl_->outline.push_back({title, page, top, std::max(0, level)});
    }

    void pdf::metadata(const std::string &key, const std::string &value) {
        impl_->check_open();
        impl_->info[key] = value;
    }
}
