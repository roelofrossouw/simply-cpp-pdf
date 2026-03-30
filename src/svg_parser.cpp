#include <svg_parser.h>
#include <sc.h>
#include "svg_element.h"
#ifndef _RSIZE_T
#define _RSIZE_T
typedef ssize_t rsize_t;
#endif  /* _RSIZE_T */
#include <tinyxml2.h>

namespace sc {
    struct svg_parser_impl {
        tinyxml2::XMLDocument doc;
    };

    std::map<std::string, std::string> attributes(const tinyxml2::XMLElement *element) {
        std::map<std::string, std::string> attributes;
        auto attr = element->FirstAttribute();
        while (attr) {
            attributes[attr->Name()] = attr->Value();
            attr = attr->Next();
        }
        return attributes;
    }

    void walk(tinyxml2::XMLElement *element, svg_processor *container, svg_element *parent = nullptr) {
        if (!element) return;
        if (!parent) parent = container->root();
        const auto svg = svg_element::generator(parent, element->Name(), attributes(element));
        walk(element->FirstChildElement(), container, svg);
        walk(element->NextSiblingElement(), container, parent);
    }


    svg_parser::svg_parser(const std::string_view xml_data) : impl(new svg_parser_impl) {
        impl->doc.Parse(xml_data.data(), xml_data.size());
    }

    svg_parser::~svg_parser() {
        delete impl;
    }

    void svg_parser::process(svg_processor *container) {
        walk(impl->doc.RootElement(), container);
        container->root()->normalize();
        container->root()->do_transform();
    }
}
