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

    class color {
    public:
        double c, m, y, k;

        color(double c, double m, double y, double k);

        color(double r, double g, double b);

        static color from_web(int r, int g, int b, int a);
    };
} // sc

#endif //BASE_TYPES_H
