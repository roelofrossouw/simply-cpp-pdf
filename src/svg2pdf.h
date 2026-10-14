#ifndef SC_SVG2PDF_H
#define SC_SVG2PDF_H
#include <memory>
#include <string_view>
#include "svg_parser.h"

namespace sc {
    struct svg2pdf_impl;

    class svg2pdf : public svg_processor {
    public:
        explicit svg2pdf(std::string_view xml_data);

        ~svg2pdf() override;

        svg2pdf(const svg2pdf &) = delete;

        svg2pdf &operator=(const svg2pdf &) = delete;

        svg_element *root() override;

        friend std::ostream &operator<<(std::ostream &lhs, const svg2pdf &rhs);

        void draw(XObjectContentContext * ctx) const;

        void resize(double new_width, double new_height, bool keep_aspect = true);

        void scale(double x, double y);

        void move(double x, double y);

        void do_transform();

        void flip();

        double height() const;

        double width() const;

    private:
        std::unique_ptr<svg2pdf_impl> impl;

    };
} // sc

#endif //SC_SVG2PDF_H
