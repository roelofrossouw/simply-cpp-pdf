#ifndef SC_SVG2PDF_H
#define SC_SVG2PDF_H
#include <string_view>
#include "svg_parser.h"

namespace sc {
    struct svg2pdf_impl;

    class svg2pdf : public svg_processor {
    public:
        explicit svg2pdf(std::string_view xml_data);

        ~svg2pdf() override;

        svg_element *root() override;

        friend std::ostream &operator<<(std::ostream &lhs, const svg2pdf &rhs);

        void draw(XObjectContentContext * ctx) const;

        void resize(double new_width, double new_height, bool keep_aspect = true);

        void scale(double x, double y);

        void move(double x, double y);

        void do_transform();

        void flip();

    private:
        svg2pdf_impl *impl;

        // void process_path_stroke(double c, double m, double y, double k, double a);
        //
        // void process_path_fill(double c, double m, double y, double k, double a);
        //
        // void process_path_move(double x, double y);
        //
        // void process_path_line(double x, double y);
        //
        // void process_path_curve(double c1x, double c1y, double c2x, double c2y, double x, double y);
        //
        // void process_path_close();
        //
        // void process_path_end();
        //
        // void process_command(path_command &cmd);
    };
} // sc

#endif //SC_SVG2PDF_H
