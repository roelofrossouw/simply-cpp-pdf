#include "../include/base_types.h"

#include <algorithm>
#include <iostream>
#include <ostream>

namespace sc {
    rect::rect(const double left, const double top, const double width, const double height) : x(left), y(top), w(width), h(height) {
    }

    rect::rect() : x(0), y(0), w(0), h(0) {
    }

    rect::rect(const rect &r) : x(r.x), y(r.y), w(r.w), h(r.h) {
    }

    rect &rect::operator=(const rect &r) {
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;
        return *this;
    }

    double rect::left() const {
        return x;
    }

    double rect::top() const {
        return y;
    }

    double rect::width() const {
        return w;
    }

    double rect::height() const {
        return h;
    }

    double rect::right() const {
        return x + w;
    }

    double rect::bottom() const {
        return y + h;
    }

    rect &rect::operator+=(const rect &rect) {
        x += rect.x;
        y += rect.y;
        w += rect.w;
        h += rect.h;
        return *this;
    }

    rect &rect::operator-=(const rect &rect) {
        x -= rect.x;
        y -= rect.y;
        w -= rect.w;
        h -= rect.h;
        return *this;
    }

    rect rect::operator+(int i) const {
        return {x - i, y - i, w + i + i, h + i + i};
    }

    rect rect::operator-(int i) const {
        return {x + i, y + i, w - i - i, h - i - i};
    }

    double rect::middle() const {
        return y + h / 2;
    }

    double rect::center() const {
        return x + w / 2;
    }

    color::color(double c, double m, double y, double k) : c(c), m(m), y(y), k(k) {
    }

    color::color(double r, double g, double b) {
        k = 1 - std::max(r, std::max(g, b));
        if (k == 1) {
            c = 0;
            m = 0;
            y = 0;
        } else {
            c = (1 - r - k) / (1 - k);
            m = (1 - g - k) / (1 - k);
            y = (1 - b - k) / (1 - k);
        }
    }

    color color::from_web(const int r, const int g, const int b, const int a = 255) {
        float alpha = a / 255.0;
        return {
            1 - ((255 - r) * alpha / 255),
            1 - ((255 - g) * alpha / 255.0),
            1 - ((255 - b) * alpha / 255.0)
        };
    }
} // rjr
