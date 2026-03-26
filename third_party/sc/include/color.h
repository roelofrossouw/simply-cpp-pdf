#ifndef SC_COLOR_H
#define SC_COLOR_H
#include <string>
#include <unordered_map>

namespace sc {
    class color {
        double c, m, y, k;
        double r, g, b, a;
        static const std::unordered_map<std::string, std::tuple<int, int, int> > NamedColors;

    public:
        color() = default;

        color(double red, double green, double blue, double alpha = 1);

        color(const std::string &color_name);

        static color from_cmyk(double cyan, double magenta, double yellow, double black, double alpha);

        static color from_web(int red, int green, int blue, double alpha = 1);

        static color from_hex(const std::string &hex);

        static color from_hsl(double x, double s, double l, double a);

        static color from_string(const std::string &str);

        double cyan() const;

        double yellow() const;

        double magenta() const;

        double black() const;

        double red() const;

        double green() const;

        double blue() const;

        double alpha() const;

        void flatten(const color &background);

        bool operator!=(const color & c2) const;

        bool operator==(const color & c2) const;

        // Standard color constants
        static const color Red;
        static const color Green;
        static const color Blue;
        static const color Black;
        static const color White;
        static const color Transparent;
    };
} // sc

#endif //SC_COLOR_H
