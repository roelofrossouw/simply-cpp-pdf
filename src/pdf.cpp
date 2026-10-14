#include "pdf.h"

#include "font_collection.h"

#include <PDFPage.h>
#include <PDFUsedFont.h>
#include <PDFWriter.h>
#include <PageContentContext.h>
#include <ResourcesDictionary.h>
#include <XObjectContentContext.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sc {
    namespace {
        std::pair<double, double> dimensions(const page_size size) {
            switch (size) {
                case page_size::a5: return {420, 595};
                case page_size::letter: return {612, 792};
                case page_size::legal: return {612, 1008};
                case page_size::a4: break;
            }
            return {595, 842};
        }

        // A height from the font's OS/2 table, in points; when the font doesn't give it (DejaVu, for
        // one), how far letter rises above the baseline.
        double glyph_height(PDFUsedFont *font, const std::pair<bool, FT_Short> &declared, const char letter, const double size) {
            if (declared.first && declared.second > 0) return declared.second * size / 1000; // thousandths of the size
            FT_Face face = *font->GetFreeTypeFont();
            if (!face || face->units_per_EM == 0 || FT_Load_Char(face, letter, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING) != 0) return 0;
            return static_cast<double>(face->glyph->metrics.horiBearingY) * size / face->units_per_EM;
        }

        // Paints the path that path() writes: filled with fill and outlined with stroke, width wide,
        // leaving out whichever of the two is transparent. Shared by every shape.
        template<typename Path>
        void fill_and_stroke(PageContentContext &content, const color &stroke, const color &fill, const double width, Path path) {
            const bool stroked = stroke.alpha() > 0 && width > 0, filled = fill.alpha() > 0;
            content.q();
            if (filled) content.k(fill.cyan(), fill.magenta(), fill.yellow(), fill.black());
            if (stroked) {
                content.w(width);
                content.K(stroke.cyan(), stroke.magenta(), stroke.yellow(), stroke.black());
            }
            path(content);
            if (stroked && filled) content.B();
            else if (filled) content.f();
            else content.S();
            content.Q();
        }

        // Control point distance for a quarter circle drawn as one cubic Bezier curve, as a fraction
        // of its radius.
        constexpr double quarter_circle = 0.5522847498;
    }

    struct pdf::impl {
        struct page_state {
            PDFPage *page;
            PageContentContext *content;
            // PDFHummus attaches links to the next page it writes, so they wait here until then.
            std::vector<std::pair<std::string, rect> > links;
        };

        PDFWriter writer;
        font_collection fonts{writer};
        double width;
        double height;
        std::vector<page_state> pages;
        unsigned current = 0; // index into pages
        bool closed = false;

        impl(const std::filesystem::path &filename, const page_size size) {
            std::tie(width, height) = dimensions(size);
            if (writer.StartPDF(filename.string(), ePDFVersion14, LogConfiguration::DefaultLogConfiguration(),
                                PDFCreationSettings(true, true)) != PDFHummus::eSuccess)
                throw std::runtime_error("sc::pdf: can't create " + filename.string());
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
            bool written = true;
            for (auto &[page, content, links]: pages) {
                written = writer.EndPageContentContext(content) == PDFHummus::eSuccess && written;
                for (const auto &[url, area]: links)
                    writer.AttachURLLinktoCurrentPage(url, PDFRectangle(area.left(), y(area.bottom()), area.right(), y(area.top())));
                written = writer.WritePageAndRelease(page) == PDFHummus::eSuccess && written;
            }
            pages.clear();
            if (writer.EndPDF() != PDFHummus::eSuccess || !written) throw std::runtime_error("sc::pdf: writing the file failed");
        }
    };

    pdf::pdf(const std::filesystem::path &filename, const page_size size) : impl_(std::make_unique<impl>(filename, size)) {
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

    void pdf::paint_text(const text &sample, const color &colour) {
        auto &content = impl_->content();
        PDFUsedFont *used = impl_->fonts.get(sample.font);
        content.BT();
        content.k(colour.cyan(), colour.magenta(), colour.yellow(), colour.black());
        content.Tf(used, sample.font.size);
        content.Td(sample.position.x(), impl_->y(sample.position.y()));
        content.Tj(sample.content);
        content.ET();
    }

    void pdf::paint_line(const point &from, const point &to, const color &colour, const double width) {
        auto &content = impl_->content();
        content.q();
        content.w(width);
        content.K(colour.cyan(), colour.magenta(), colour.yellow(), colour.black());
        content.m(from.x(), impl_->y(from.y()));
        content.l(to.x(), impl_->y(to.y()));
        content.S();
        content.Q();
    }

    void pdf::paint_rounded_rect(const rect &shape, const double radius, const color &stroke, const color &fill, const double width) {
        const double left = shape.left(), right = shape.right(), bottom = impl_->y(shape.bottom()), top = impl_->y(shape.top());
        fill_and_stroke(impl_->content(), stroke, fill, width, [&](PageContentContext &content) {
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

    void pdf::paint_circle(const circle &shape, const color &stroke, const color &fill, const double width) {
        const double x = shape.center().x(), y = impl_->y(shape.center().y()), r = shape.radius();
        const double k = r * quarter_circle; // control point distance from the end points
        fill_and_stroke(impl_->content(), stroke, fill, width, [&](PageContentContext &content) {
            content.m(x + r, y);
            content.c(x + r, y + k, x + k, y + r, x, y + r);
            content.c(x - k, y + r, x - r, y + k, x - r, y);
            content.c(x - r, y - k, x - k, y - r, x, y - r);
            content.c(x + k, y - r, x + r, y - k, x + r, y);
            content.h();
        });
    }

    void pdf::paint_polygon(const polygon &shape, const color &stroke, const color &fill, const double width) {
        fill_and_stroke(impl_->content(), stroke, fill, width, [&](PageContentContext &content) {
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

    void pdf::paint_path(const path &shape, const color &stroke, const color &fill, const double width) {
        auto &content = impl_->content();
        const bool stroked = stroke.alpha() > 0 && width > 0, filled = fill.alpha() > 0;
        content.q();
        if (filled) content.k(fill.cyan(), fill.magenta(), fill.yellow(), fill.black());
        if (stroked) {
            content.w(width);
            content.K(stroke.cyan(), stroke.magenta(), stroke.yellow(), stroke.black());
        }
        trace(content, shape, impl_->height);
        if (stroked && filled) shape.even_odd ? content.BStar() : content.B();
        else if (filled) shape.even_odd ? content.fStar() : content.f();
        else content.S();
        content.Q();
    }

    void pdf::push_clip(const path &shape) {
        auto &content = impl_->content();
        content.q();
        trace(content, shape, impl_->height);
        shape.even_odd ? content.WStar() : content.W();
        content.n();
    }

    void pdf::pop_clip() {
        impl_->content().Q();
    }

    void pdf::add_link(const std::string &url, const rect &pos) {
        impl_->check_open();
        impl_->pages[impl_->current].links.emplace_back(url, pos);
    }
}
