#include "pdf.h"

#include <pdflite.h>
#include <standard_font.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace sc {
    struct pdf::impl {
        pdflite::document document;
        bool builtin = false;
        // One pdflite::font serves every size of a face: keyed by family, weight, italic and style.
        std::map<std::tuple<std::string, int, bool, std::string>, pdflite::font> fonts;
        // Each bitmap read once, by its bytes (copies share them), held so their address isn't another's.
        std::map<const std::string *, std::pair<sc::bitmap, std::shared_ptr<const pdflite::image> > > bitmaps;
        // Each image file read once.
        std::map<std::filesystem::path, std::shared_ptr<const pdflite::image> > files;

        impl(const std::filesystem::path &filename, const page_size size, const orientation turned) : document(filename, size, turned) {
        }

        void check_open() const {
            if (document.closed()) throw std::logic_error("sc::pdf: drawing after close()");
        }

        pdflite::content &content() {
            check_open();
            return document.page_content();
        }

        // The installed font for spec; throws std::runtime_error when fontconfig finds none.
        const pdflite::font &font(const sc::font &spec) {
            const auto key = std::tuple{spec.family, spec.weight, spec.italic, spec.style};
            if (const auto found = fonts.find(key); found != fonts.end()) return found->second;
            return fonts.emplace(key, pdflite::font{spec}).first->second;
        }

        // Makes what follows (up to the next restore) paint at these alphas; opaque needs nothing.
        void set_alpha(pdflite::content &page, const double fill_alpha, const double stroke_alpha) {
            if (fill_alpha >= 1 && stroke_alpha >= 1) return;
            page.state(document.graphics_state(std::clamp(fill_alpha, 0.0, 1.0), std::clamp(stroke_alpha, 0.0, 1.0)));
        }

        // Fills (by the fill's rule) and/or strokes what path() writes; whichever is none is left out.
        template<typename Path>
        void paint(const sc::appearance &look, Path path) {
            const auto &stroke = look.stroke();
            const auto &fill = look.fill();
            auto &page = content();
            const bool stroked = stroke.visible(), filled = fill.visible();
            const bool even_odd = fill.rule() == fill_rule::even_odd;
            page.save();
            set_alpha(page, filled ? fill.colour().alpha() : 1, stroked ? stroke.colour().alpha() : 1);
            if (filled) page.fill_colour(fill.colour().red(), fill.colour().green(), fill.colour().blue());
            if (stroked) {
                page.line_width(stroke.width());
                page.stroke_colour(stroke.colour().red(), stroke.colour().green(), stroke.colour().blue());
            }
            path(page);
            if (stroked && filled) page.fill_and_stroke(even_odd);
            else if (filled) page.fill(even_odd);
            else if (stroked) page.stroke();
            else page.end_path();
            page.restore();
        }

        // A JPEG or PNG, by its first bytes.
        static std::shared_ptr<const pdflite::image> picture(std::string bytes) {
            if (bytes.starts_with("\x89PNG")) return std::make_shared<pdflite::png>(std::move(bytes));
            if (bytes.starts_with("\xFF\xD8")) return std::make_shared<pdflite::jpeg>(std::move(bytes));
            throw std::runtime_error("sc::pdf: an image that's neither a JPEG nor a PNG");
        }
    };

    pdf::pdf(const std::filesystem::path &filename, const page_size size, const orientation turned)
        : impl_(std::make_unique<impl>(filename, size, turned)) {
    }

    pdf::pdf(const std::filesystem::path &filename, const page_size size, const orientation turned, const protection &locked)
        : impl_(std::make_unique<impl>(filename, size, turned)) {
        if (locked.user_password.empty() && locked.owner_password.empty()) return;
        pdflite::permissions allowed;
        allowed.print = allowed.print_high_quality = locked.print;
        allowed.copy = locked.copy;
        allowed.modify = locked.edit;
        impl_->document.protect({locked.user_password, locked.owner_password, allowed});
    }

    pdf::~pdf() {
        if (!impl_) return;
        try {
            impl_->document.close();
        } catch (const std::exception &error) {
            std::cerr << error.what() << '\n';
        }
    }

    pdf::pdf(pdf &&) noexcept = default;

    pdf &pdf::operator=(pdf &&other) noexcept {
        if (this != &other) {
            if (impl_) {
                try {
                    impl_->document.close();
                } catch (const std::exception &error) {
                    std::cerr << error.what() << '\n';
                }
            }
            impl_ = std::move(other.impl_);
        }
        return *this;
    }

    void pdf::close() {
        impl_->document.close();
    }

    void pdf::new_page() {
        impl_->check_open();
        impl_->document.add_page();
    }

    void pdf::set_page(const unsigned page) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::set_page: pages count from 1");
        while (impl_->document.page_count() < page) impl_->document.add_page();
        impl_->document.set_page(page);
    }

    unsigned pdf::page() const {
        return static_cast<unsigned>(impl_->document.page());
    }

    unsigned pdf::page_count() const {
        return static_cast<unsigned>(impl_->document.page_count());
    }

    double pdf::page_width() const {
        return impl_->document.size().width().points();
    }

    double pdf::page_height() const {
        return impl_->document.size().height().points();
    }

    namespace {
        // UTF-8 text in pieces: Windows-1252's (as its bytes) and the rest (as UTF-8), in order.
        struct piece {
            std::string text; // Windows-1252 bytes when encoded, else UTF-8
            std::string utf8;
            bool encoded;
        };

        std::vector<piece> pieces_of(const std::string_view text) {
            std::vector<piece> out;
            for (std::size_t i = 0; i < text.size();) {
                const auto first = static_cast<unsigned char>(text[i]);
                const std::size_t length = first < 0x80 ? 1 : (first >> 5) == 6 ? 2 : (first >> 4) == 14 ? 3 : (first >> 3) == 30 ? 4 : 1;
                char32_t code = length == 1 ? first : first & (0x7F >> length);
                for (std::size_t k = 1; k < length && i + k < text.size(); ++k) code = (code << 6) | (static_cast<unsigned char>(text[i + k]) & 0x3F);
                const auto byte = standard_font::encode(code);
                const bool encoded = byte.has_value();
                if (out.empty() || out.back().encoded != encoded) out.push_back({{}, {}, encoded});
                if (encoded) out.back().text += *byte;
                else out.back().text.append(text.substr(i, length));
                out.back().utf8.append(text.substr(i, length));
                i += length;
            }
            return out;
        }
    }

    double pdf::measure_width(const text &sample) {
        if (impl_->builtin) {
            if (const auto standard = standard_font::of(sample.font)) {
                double width = 0;
                for (const auto &each: pieces_of(sample.content))
                    width += each.encoded ? standard->width(each.utf8, sample.font.size)
                                          : impl_->font(sample.font).width(each.text, sample.font.size).points();
                return width;
            }
        }
        return impl_->font(sample.font).width(sample.content, sample.font.size).points();
    }

    double pdf::measure_x_height(const font &font) {
        if (impl_->builtin)
            if (const auto standard = standard_font::of(font)) return standard->x_height(font.size);
        return impl_->font(font).x_height(font.size).points();
    }

    double pdf::measure_cap_height(const font &font) {
        if (impl_->builtin)
            if (const auto standard = standard_font::of(font)) return standard->cap_height(font.size);
        return impl_->font(font).cap_height(font.size).points();
    }

    void pdf::builtin_fonts(const bool on) { impl_->builtin = on; }

    bool pdf::builtin_fonts() const { return impl_->builtin; }

    void pdf::paint_text(const text &sample, const sc::fill &fill) {
        auto &content = impl_->content();
        auto &document = impl_->document;
        content.save();
        impl_->set_alpha(content, fill.colour().alpha(), 1);
        content.fill_colour(fill.colour().red(), fill.colour().green(), fill.colour().blue());
        content.begin_text();
        const auto standard = impl_->builtin ? standard_font::of(sample.font) : std::nullopt;
        if (standard) {
            // The built-in font for what Windows-1252 has, placed by Adobe's widths; the installed
            // font (embedded) for the rest.
            double x = sample.position.x();
            for (const auto &each: pieces_of(sample.content)) {
                if (each.encoded) {
                    content.font(document.standard_font(standard->which()), sample.font.size);
                    content.text_position(x, sample.position.y());
                    content.show(each.text);
                    x += standard->width(each.utf8, sample.font.size);
                } else {
                    const auto &installed = impl_->font(sample.font);
                    content.font(document.font(installed), sample.font.size);
                    content.text_position(x, sample.position.y());
                    content.show(installed.encode(each.text));
                    x += installed.width(each.text, sample.font.size).points();
                }
            }
        } else {
            const auto &installed = impl_->font(sample.font);
            content.font(document.font(installed), sample.font.size);
            content.text_position(sample.position.x(), sample.position.y());
            content.show(installed.encode(sample.content));
        }
        content.end_text();
        content.restore();
    }

    void pdf::paint_line(const line &shape, const sc::stroke &stroke) {
        impl_->paint(stroke, [&](pdflite::content &content) {
            content.move(shape.from().x(), shape.from().y());
            content.line(shape.to().x(), shape.to().y());
        });
    }

    void pdf::paint_rounded_rect(const rounded_rect &shape, const sc::appearance &look) {
        const rect box = shape.rect();
        const double radius = shape.radius();
        const double left = box.left(), right = box.right(), top = box.top(), bottom = box.bottom();
        impl_->paint(look, [&](pdflite::content &content) {
            const double control = radius * (1 - quarter_circle); // from the corner
            content.move(left + radius, top);
            content.line(right - radius, top);
            content.curve(right - control, top, right, top + control, right, top + radius);
            content.line(right, bottom - radius);
            content.curve(right, bottom - control, right - control, bottom, right - radius, bottom);
            content.line(left + radius, bottom);
            content.curve(left + control, bottom, left, bottom - control, left, bottom - radius);
            content.line(left, top + radius);
            content.curve(left, top + control, left + control, top, left + radius, top);
            content.close_path();
        });
    }

    void pdf::paint_circle(const circle &shape, const sc::appearance &look) {
        const double x = shape.center().x(), y = shape.center().y(), r = shape.radius();
        const double k = r * quarter_circle; // control point distance from the end points (sc-core's path.h)
        impl_->paint(look, [&](pdflite::content &content) {
            content.move(x + r, y);
            content.curve(x + r, y + k, x + k, y + r, x, y + r);
            content.curve(x - k, y + r, x - r, y + k, x - r, y);
            content.curve(x - r, y - k, x - k, y - r, x, y - r);
            content.curve(x + k, y - r, x + r, y - k, x + r, y);
            content.close_path();
        });
    }

    void pdf::paint_polygon(const polygon &shape, const sc::appearance &look) {
        impl_->paint(look, [&](pdflite::content &content) {
            content.move(shape[0].x(), shape[0].y());
            for (std::size_t i = 1; i < shape.size(); ++i) content.line(shape[i].x(), shape[i].y());
            content.close_path();
        });
    }

    void pdf::draw_image(const std::filesystem::path &filename, const rect &pos) {
        impl_->check_open();
        auto found = impl_->files.find(filename);
        if (found == impl_->files.end()) {
            std::ifstream in{filename, std::ios::binary};
            if (!in) throw std::runtime_error("sc::pdf: can't read the image " + filename.string());
            std::string bytes{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
            found = impl_->files.emplace(filename, impl::picture(std::move(bytes))).first;
        }
        const auto &picture = *found->second;
        const auto [image_width, image_height] = std::pair{picture.size().width(), picture.size().height()};
        if (image_width <= 0 || image_height <= 0) throw std::runtime_error("sc::pdf: can't read the image " + filename.string());
        const double scale = std::min(pos.width() / image_width, pos.height() / image_height);
        const double w = image_width * scale, h = image_height * scale;
        const rect where{pos.left() + (pos.width() - w) / 2, pos.top() + (pos.height() - h) / 2, w, h};
        impl_->content().draw(impl_->document.image(picture), where);
    }

    void pdf::paint_bitmap(const bitmap &picture, const transform &placement) {
        impl_->check_open();
        auto found = impl_->bitmaps.find(&picture.bytes());
        if (found == impl_->bitmaps.end())
            found = impl_->bitmaps.emplace(&picture.bytes(), std::pair{picture, impl::picture(picture.bytes())}).first;
        impl_->content().draw(impl_->document.image(*found->second.second), placement);
    }

    namespace {
        // Writes shape's steps into content.
        void trace(pdflite::content &content, const path &shape) {
            for (const auto &step: shape.steps()) {
                switch (step.type) {
                    case path::kind::move: content.move(step.to.x(), step.to.y()); break;
                    case path::kind::line: content.line(step.to.x(), step.to.y()); break;
                    case path::kind::cubic:
                        content.curve(step.first.x(), step.first.y(), step.second.x(), step.second.y(), step.to.x(), step.to.y());
                        break;
                    case path::kind::close: content.close_path(); break;
                }
            }
        }
    }

    void pdf::paint_path(const path &shape, const sc::appearance &look) {
        impl_->paint(look, [&](pdflite::content &content) { trace(content, shape); });
    }

    void pdf::push_clip(const path &shape) {
        auto &content = impl_->content();
        content.save();
        trace(content, shape);
        content.clip(); // nonzero, as canvas::push_clip() clips
        content.end_path();
    }

    void pdf::pop_clip() {
        impl_->content().restore();
    }

    void pdf::add_link(const std::string &url, const rect &pos) {
        impl_->check_open();
        impl_->document.link(pos, url);
    }

    void pdf::add_link(const unsigned page, const rect &pos, const double top) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::add_link: pages count from 1");
        impl_->document.link(pos, static_cast<std::size_t>(page), top);
    }

    void pdf::add_outline(const std::string &title, const unsigned page, const double top, const int level) {
        impl_->check_open();
        if (page == 0) throw std::out_of_range("sc::pdf::add_outline: pages count from 1");
        impl_->document.bookmark(title, page, top, std::max(0, level));
    }

    void pdf::metadata(const std::string &key, const std::string &value) {
        impl_->check_open();
        impl_->document.info(key, value);
    }
}
