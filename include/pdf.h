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

    // Writes a PDF file: pages of text, shapes, images, SVG drawings and links.
    //
    // An sc::canvas: shapes and text are drawn with its draw() calls, the same as on an sc::image.
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

        // How far text in that font advances, in points. Throws std::runtime_error, as drawing text
        // does, when no font is found.
        [[nodiscard]] double text_width(std::string_view content, const font &font = {});

        // The height of a lowercase x and of a capital H in that font, in points: what the font
        // declares, or else the letter's own height.
        [[nodiscard]] double text_x_height(const font &font = {});

        [[nodiscard]] double text_cap_height(const font &font = {});

        // A JPEG, PNG or TIFF image, scaled to fit pos with its aspect ratio kept and centred in it.
        // Throws std::runtime_error when the file can't be read as an image.
        void draw_image(const std::filesystem::path &filename, const rect &pos);

        // An SVG drawing, scaled to fit pos with its aspect ratio kept and centred in it, as vector
        // graphics: paths, shapes, groups, transforms, clip paths and <use>, with flat colours.
        void draw_svg(std::string_view svg, const rect &pos);

        // Makes pos on the current page a link to url.
        void add_link(const std::string &url, const rect &pos);

    protected:
        // Self-intersecting polygons fill by the nonzero winding rule. Text throws
        // std::runtime_error when no font is found.
        void paint_polygon(const polygon &shape, const color &stroke, const color &fill, double width) override;

        void paint_circle(const circle &shape, const color &stroke, const color &fill, double width) override;

        void paint_line(const point &from, const point &to, const color &colour, double width) override;

        void paint_text(const text &sample, const color &colour) override;

        // Real quarter circles, not the polygon a canvas draws by default.
        void paint_rounded_rect(const rect &shape, double radius, const color &stroke, const color &fill, double width) override;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };
}

#endif //SC_PDF_H
