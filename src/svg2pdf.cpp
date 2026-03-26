#include "svg2pdf.h"
#include "svg_element.h"
#include <iostream>


namespace sc {
    struct svg2pdf_impl {
        svg_element *root;

        svg2pdf_impl() : root(new svg_xml()) {
        }

        ~svg2pdf_impl() {
            delete root;
        }
    };

    svg_element *svg2pdf::root() {
        return impl->root;
    }

    void svg2pdf::draw(XObjectContentContext *ctx) const {
        impl->root->draw(ctx);
    }

    void svg2pdf::resize(int new_width, int new_height, bool keep_aspect) {
        impl->root->resize(new_width, new_height, keep_aspect);
    }

    void svg2pdf::scale(int x, int y) {
        impl->root->scale(x, y);
    }

    void svg2pdf::move(double x, double y) {
        impl->root->normalize({x, y});
    }

    svg2pdf::svg2pdf(const std::string_view xml_data) : impl(new svg2pdf_impl) {
        svg_parser p(xml_data);
        p.process(this);
    }

    svg2pdf::~svg2pdf() {
        delete impl;
    }

    std::ostream &operator<<(std::ostream &lhs, const svg2pdf &rhs) {
        lhs << *rhs.impl->root << std::endl;
        return lhs;
    }
} // sc
