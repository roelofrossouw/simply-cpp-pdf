#ifndef PARSESVG_SVGPARSER_H
#define PARSESVG_SVGPARSER_H
#include <string_view>
#include "svg_element.h"

namespace sc {
    struct svg_parser_impl;

    class svg_processor {
    public:
        virtual ~svg_processor() = default;

        virtual svg_element *root() = 0;
    };

    class svg_parser {
    public:
        svg_parser(std::string_view xml_data);

        ~svg_parser();

        void process(svg_processor *container);

    private:
        svg_parser_impl *impl;
    };
}

#endif //PARSESVG_SVGPARSER_H
