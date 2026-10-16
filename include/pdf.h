#ifndef SC_PDF_H
#define SC_PDF_H

#include <canvas.h>
#include <font.h>
#include <rect.h>
#include <text.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace sc {
    // Page sizes, in points (1/72 inch): A4 595x842, A5 420x595, Letter 612x792, Legal 612x1008.
    enum class page_size { a4, a5, letter, legal };

    // Writes a PDF file: pages of text, shapes, images, SVG drawings (canvas::draw(sc::svg, rect))
    // and links.
    //
    // An sc::canvas: shapes and text are drawn with its draw() calls, and text measured with its
    // text_width(), text_x_height() and text_cap_height(), the same as on an sc::image.
    // Positions are in points from the page's top-left corner, x to the right and y down, as on a
    // screen (PDF itself counts y up from the bottom; this converts). Fonts are found through
    // fontconfig and embedded. Drawing goes to the current page, which is the newest one unless
    // set_page() moved back to an earlier one. The file is finished by close(), or by the
    // destructor, which can't report a failure.
    class pdf : public canvas {
    public:
        // Starts the file, with one page; throws std::runtime_error when it can't be created.
        explicit pdf(const std::filesystem::path &filename, page_size size = page_size::a4);

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

        [[nodiscard]] double page_width() const;

        [[nodiscard]] double page_height() const;

        // A JPEG, PNG or TIFF image, scaled to fit pos with its aspect ratio kept and centred in it.
        // Throws std::runtime_error when the file can't be read as an image.
        void draw_image(const std::filesystem::path &filename, const rect &pos);

        // Makes pos on the current page a link to url.
        void add_link(const std::string &url, const rect &pos);

    protected:
        // Strokes are a colour and a width, fills a colour and a fill rule (even-odd or nonzero), all
        // opaque; other stroke and fill styles aren't drawn yet. Text throws std::runtime_error when
        // no font is found.
        void paint_polygon(const polygon &shape, const sc::stroke &stroke, const sc::fill &fill) override;

        void paint_circle(const circle &shape, const sc::stroke &stroke, const sc::fill &fill) override;

        void paint_line(const line &shape, const sc::stroke &stroke) override;

        void paint_text(const text &sample, const sc::fill &fill) override;

        // In points, in the font fontconfig finds (std::runtime_error when there's none). x and cap
        // heights are what the font declares, or else the letter's own height.
        double measure_width(const text &sample) override;

        double measure_x_height(const font &font) override;

        double measure_cap_height(const font &font) override;

        // Real quarter circles, not the polygon a canvas draws by default.
        void paint_rounded_rect(const rounded_rect &shape, const sc::stroke &stroke, const sc::fill &fill) override;

        // Real curves; and clipping (so SVG clip paths work).
        void paint_path(const path &shape, const sc::stroke &stroke, const sc::fill &fill) override;

        void push_clip(const path &shape) override;

        void pop_clip() override;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };
}

#endif //SC_PDF_H
