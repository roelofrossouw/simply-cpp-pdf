#include "rect.h"

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

    rect rect::operator+(const rect &r) const {
        return {x + r.x, y + r.y, w + r.w, h + r.h};
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
}
