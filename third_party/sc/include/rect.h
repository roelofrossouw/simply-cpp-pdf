#ifndef SC_RECT_H
#define SC_RECT_H

namespace sc {
    class rect {
    public:
        rect(double left, double top, double width = 0, double height = 0);

        rect();

        rect(const rect &r);

        rect &operator=(const rect &r);

        [[nodiscard]] double left() const;

        [[nodiscard]] double bottom() const;

        [[nodiscard]] double width() const;

        [[nodiscard]] double height() const;

        [[nodiscard]] double right() const;

        [[nodiscard]] double top() const;

        rect &operator+=(const rect &rect);

        rect &operator-=(const rect &rect);

        rect operator+(int i) const;

        rect operator+(const rect &r) const;

        rect operator-(int i) const;

        double middle() const;

        double center() const;

    protected:
        double x, y, w, h;
    };
} // sc

#endif //SC_RECT_H
