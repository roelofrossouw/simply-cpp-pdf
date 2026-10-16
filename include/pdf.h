#ifndef SC_PDF_H
#define SC_PDF_H

#include <canvas.h>
#include <font.h>
#include <page.h>
#include <rect.h>
#include <text.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace sc {
    // Writes a PDF file on PDFHummus: pages of text, shapes, images, SVG drawings
    // (canvas::draw(sc::svg, rect)) and links.
    //
    // An sc::canvas, drawn and measured with the same calls as an sc::image. Positions are in points
    // (1/72 inch) from the page's top-left corner, y down (PDF counts y up; this converts). A
    // colour's alpha is honoured. Fonts are found through fontconfig and embedded. Drawing goes to
    // the current page: the newest, unless set_page() moved back. The file is finished by close(),
    // or by the destructor, which can't report a failure.
    class pdf : public canvas {
    public:
        // Starts the file with one page; every page has this size (sc-graphics' page.h: A3, A4, A5,
        // Letter, Legal) and orientation. Throws std::runtime_error when the file can't be created.
        explicit pdf(const std::filesystem::path &filename, page_size size = page_size::a4, orientation turned = orientation::portrait);

        ~pdf() override;

        pdf(const pdf &) = delete;

        pdf &operator=(const pdf &) = delete;

        pdf(pdf &&) noexcept;

        pdf &operator=(pdf &&) noexcept;

        // Writes out the pages and finishes the file; throws std::runtime_error when that fails.
        // Nothing can be drawn afterwards. Calling it again does nothing.
        void close();

        // Adds a page after the last one and makes it current.
        void new_page();

        // Makes page (counting from 1) current, adding pages up to it when there are fewer.
        void set_page(unsigned page);

        [[nodiscard]] unsigned page() const;

        [[nodiscard]] unsigned page_count() const;

        // In points, landscape already turned.
        [[nodiscard]] double page_width() const;

        [[nodiscard]] double page_height() const;

        // A JPEG, PNG or TIFF image, scaled to fit pos with its aspect ratio kept and centred in it.
        // Throws std::runtime_error when the file can't be read as an image.
        void draw_image(const std::filesystem::path &filename, const rect &pos);

        // Makes pos on the current page a link to url.
        void add_link(const std::string &url, const rect &pos);

    protected:
        // Strokes are a colour and a width, fills a colour and a fill rule (even-odd or nonzero);
        // alpha below 1 goes through a graphics state (ExtGState). Other stroke and fill styles
        // aren't drawn yet. Text throws std::runtime_error when no font is found.
        void paint_polygon(const polygon &shape, const sc::appearance &look) override;

        void paint_circle(const circle &shape, const sc::appearance &look) override;

        void paint_line(const line &shape, const sc::stroke &stroke) override;

        void paint_text(const text &sample, const sc::fill &fill) override;

        // In points, in the font fontconfig finds (std::runtime_error when there's none). x and cap
        // heights are what the font declares, or else the letter's own height.
        double measure_width(const text &sample) override;

        double measure_x_height(const font &font) override;

        double measure_cap_height(const font &font) override;

        // Real quarter circles, not the polygon a canvas draws by default.
        void paint_rounded_rect(const rounded_rect &shape, const sc::appearance &look) override;

        // Real curves; clips are nonzero, nest, and make SVG clip paths work.
        void paint_path(const path &shape, const sc::appearance &look) override;

        void push_clip(const path &shape) override;

        void pop_clip() override;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };
}

#endif //SC_PDF_H
