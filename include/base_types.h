#ifndef BASE_TYPES_H
#define BASE_TYPES_H

namespace sc {
    class rect {
    public:
        rect(double left, double bottom, double width = 0, double height = 0);

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

        rect operator-(int i) const;

        double middle() const;

        double center() const;

    protected:
        double x, y, w, h;
    };
} // sc

#endif //BASE_TYPES_H
