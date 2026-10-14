#ifndef SC_PDF_H
#define SC_PDF_H

#include "font_spec.h"

#include <circle.h>
#include <color.h>
#include <point.h>
#include <polygon.h>
#include <rect.h>
#include <rotated_rect.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace sc {
    // Page sizes, in points (1/72 inch): A4 595x842, A5 420x595, Letter 612x792, Legal 612x1008.
    enum class page_size { a4, a5, letter, legal };

    // Writes a PDF file: pages of text, lines, rectangles, images, SVG drawings and links.
    //
    // Positions are in points from the page's top-left corner, x to the right and y down, as on a
    // screen (PDF itself counts y up from the bottom; this converts). Drawing goes to the current
    // page, which is the newest one unless set_page() moved back to an earlier one. The file is
    // finished by close(), or by the destructor, which can't report a failure.
    class pdf {
    public:
        // Starts the file, with one page; throws std::runtime_error when it can't be created.
        explicit pdf(const std::filesystem::path &filename, page_size size = page_size::a4);

        ~pdf();

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

        // Writes text with its baseline along pos.bottom(), starting at pos.left() (a rect with no
        // height puts the baseline at pos.top()). Throws std::runtime_error when no font is found.
        void draw_text(std::string_view text, const rect &pos, double size = 10, const font_spec &font = {},
                       const color &colour = color::Black);

        // How far text written in that font and size advances, in points.
        [[nodiscard]] double text_width(std::string_view text, const font_spec &font = {}, double size = 10);

        // The height of a lowercase x and of a capital H in that font and size, in points: what the
        // font declares, or else the letter's own height.
        [[nodiscard]] double text_x_height(const font_spec &font = {}, double size = 10);

        [[nodiscard]] double text_cap_height(const font_spec &font = {}, double size = 10);

        void draw_line(const point &from, const point &to, double width = 1, const color &colour = color::Black);

        // The draw() overloads take an sc-core geometry and paint its outline in stroke, width
        // points wide, and its inside in fill. A transparent colour (alpha 0, color::Transparent)
        // leaves that part out.

        // A rectangle, its corners rounded with radius when that's above 0 (at most half the
        // shorter side).
        void draw(const rect &pos, const color &stroke, const color &fill = color::Transparent, double width = 1,
                  double radius = 0);

        void draw(const circle &shape, const color &stroke, const color &fill = color::Transparent, double width = 1);

        // A closed polygon through its points (fewer than two draw nothing). Self-intersecting
        // polygons fill by the nonzero winding rule.
        void draw(const polygon &shape, const color &stroke, const color &fill = color::Transparent, double width = 1);

        // A rectangle turned angle degrees about its centre, clockwise on the page (y points down),
        // as sc-core's rotated_rect corners are.
        void draw(const rotated_rect &shape, const color &stroke, const color &fill = color::Transparent,
                  double width = 1);

        // A JPEG, PNG or TIFF image, scaled to fit pos with its aspect ratio kept and centred in it.
        // Throws std::runtime_error when the file can't be read as an image.
        void draw_image(const std::filesystem::path &filename, const rect &pos);

        // An SVG drawing, scaled to fit pos with its aspect ratio kept and centred in it, as vector
        // graphics: paths, shapes, groups, transforms, clip paths and <use>, with flat colours.
        void draw_svg(std::string_view svg, const rect &pos);

        // Makes pos on the current page a link to url.
        void add_link(const std::string &url, const rect &pos);

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };
}

#endif //SC_PDF_H
