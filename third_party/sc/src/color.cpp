#include "color.h"
#include <algorithm>
#include <iostream>
#include <regex>
#include <unordered_map>

namespace sc {
    color::color(double red, double green, double blue, double alpha) : r(red), g(green), b(blue), a(alpha) {
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

    color::color(const std::string &color_name) {
        this->operator=(from_string(color_name));
    }

    color color::from_cmyk(double cyan, double magenta, double yellow, double black, double alpha) {
        double red = (1.0 - cyan) * (1.0 - black);
        double green = (1.0 - magenta) * (1.0 - black);
        double blue = (1.0 - yellow) * (1.0 - black);
        return {red, green, blue, alpha};
    }

    color color::from_web(int red, int green, int blue, double alpha) {
        return {red / 255.0, green / 255.0, blue / 255.0, alpha};
    }

    color color::from_hex(const std::string &hex) {
        double r = 0, g = 0, b = 0;
        if (hex.size() == 7) {
            // #RRGGBB
            r = std::stoi(hex.substr(1, 2), nullptr, 16) / 255.0;
            g = std::stoi(hex.substr(3, 2), nullptr, 16) / 255.0;
            b = std::stoi(hex.substr(5, 2), nullptr, 16) / 255.0;
        } else if (hex.size() == 4) {
            // #RGB shorthand
            r = std::stoi(std::string(2, hex[1]), nullptr, 16) / 255.0;
            g = std::stoi(std::string(2, hex[2]), nullptr, 16) / 255.0;
            b = std::stoi(std::string(2, hex[3]), nullptr, 16) / 255.0;
        }
        return {r, g, b, 1};
    }

    color color::from_hsl(double h, double s, double l, double a) {
        h /= 360.0; // std::fmod(h, 360.0) / 360.0;
        s /= 100.0;
        l /= 100.0;

        auto hue2rgb = [](double p, double q, double t) {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0 / 6) return p + (q - p) * 6 * t;
            if (t < 1.0 / 2) return q;
            if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
            return p;
        };

        double r, g, b;
        if (s == 0) {
            r = g = b = l; // achromatic
        } else {
            double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            double p = 2 * l - q;
            r = hue2rgb(p, q, h + 1.0 / 3);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0 / 3);
        }
        return {r, g, b, a};
    }

    color color::from_string(const std::string &str) {
        if (str.empty()) return {0, 0, 0, 1};

        if (str[0] == '#') {
            return from_hex(str);
        }

        std::smatch match;
        // rgb() or rgba()
        if (std::regex_match(str, match, std::regex(R"(rgba?\(([^)]+)\))"))) {
            std::stringstream ss(match[1].str());
            int r, g, b;
            double a = 1.0;
            char comma;
            ss >> r >> comma >> g >> comma >> b;
            if (ss >> comma >> a) {
            } // optional alpha
            return from_web(r, g, b, a);
        }

        // hsl() or hsla()
        if (std::regex_match(str, match, std::regex(R"(hsla?\(([^)]+)\))"))) {
            std::stringstream ss(match[1].str());
            double h, s, l, a = 1.0;
            char comma, percent;
            ss >> h >> comma >> s >> percent >> comma >> l >> percent;
            if (ss >> comma >> a) {
            }
            return from_hsl(h, s, l, a);
        }


        auto it = NamedColors.find(str);
        if (it != NamedColors.end()) {
            auto [red,green,blue] = it->second;
            return from_web(red, green, blue, 1);
        }

        if (str == "transparent" || str == "none") return {0, 0, 0, 0};

        std::string lstr = str;
        std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](unsigned char c) { return std::tolower(c); });


        it = NamedColors.find(lstr);
        if (it != NamedColors.end()) {
            auto [red,green,blue] = it->second;
            return from_web(red, green, blue, 1);
        }
        if (lstr == "transparent" || lstr == "none") return {0, 0, 0, 0};

        return {0, 0, 0, 1}; // fallback: black
    }

    double color::cyan() const {
        return c;
    }

    double color::yellow() const {
        return y;
    }

    double color::magenta() const {
        return m;
    }

    double color::black() const {
        return k;
    }

    double color::red() const {
        return k == 1 ? 0 : 1 + c * k - k - c;
    }

    double color::green() const {
        return k == 1 ? 0 : 1 + m * k - k - m;
    }

    double color::blue() const {
        return k == 1 ? 0 : 1 + y * k - k - y;
    }

    double color::alpha() const {
        return a;
    }

    void color::flatten(const color &background) {
    }

    bool color::operator!=(const color &c2) const {
        return !operator==(c2);
    }

    bool color::operator==(const color &c2) const {
        return c == c2.c && y == c2.y && m == c2.m && k == c2.k && a == c2.a;
    }

    const color color::Red = color(1.0, 0.0, 0.0);
    const color color::Green = color(0.0, 1.0, 0.0);
    const color color::Blue = color(0.0, 0.0, 1.0);
    const color color::Black = color(0.0, 0.0, 0.0);
    const color color::White = color(1.0, 1.0, 1.0);
    const color color::Transparent = color(0.0, 0.0, 0.0, 0.0);

    const std::unordered_map<std::string, std::tuple<int, int, int> > color::NamedColors = {
        {"aliceblue", {240, 248, 255}},
        {"antiquewhite", {250, 235, 215}},
        {"aqua", {0, 255, 255}},
        {"aquamarine", {127, 255, 212}},
        {"azure", {240, 255, 255}},
        {"beige", {245, 245, 220}},
        {"bisque", {255, 228, 196}},
        {"black", {0, 0, 0}},
        {"blanchedalmond", {255, 235, 205}},
        {"blue", {0, 0, 255}},
        {"blueviolet", {138, 43, 226}},
        {"brown", {165, 42, 42}},
        {"burlywood", {222, 184, 135}},
        {"cadetblue", {95, 158, 160}},
        {"chartreuse", {127, 255, 0}},
        {"chocolate", {210, 105, 30}},
        {"coral", {255, 127, 80}},
        {"cornflowerblue", {100, 149, 237}},
        {"cornsilk", {255, 248, 220}},
        {"crimson", {220, 20, 60}},
        {"cyan", {0, 255, 255}},
        {"darkblue", {0, 0, 139}},
        {"darkcyan", {0, 139, 139}},
        {"darkgoldenrod", {184, 134, 11}},
        {"darkgray", {169, 169, 169}},
        {"darkgreen", {0, 100, 0}},
        {"darkgrey", {169, 169, 169}},
        {"darkkhaki", {189, 183, 107}},
        {"darkmagenta", {139, 0, 139}},
        {"darkolivegreen", {85, 107, 47}},
        {"darkorange", {255, 140, 0}},
        {"darkorchid", {153, 50, 204}},
        {"darkred", {139, 0, 0}},
        {"darksalmon", {233, 150, 122}},
        {"darkseagreen", {143, 188, 143}},
        {"darkslateblue", {72, 61, 139}},
        {"darkslategray", {47, 79, 79}},
        {"darkslategrey", {47, 79, 79}},
        {"darkturquoise", {0, 206, 209}},
        {"darkviolet", {148, 0, 211}},
        {"deeppink", {255, 20, 147}},
        {"deepskyblue", {0, 191, 255}},
        {"dimgray", {105, 105, 105}},
        {"dimgrey", {105, 105, 105}},
        {"dodgerblue", {30, 144, 255}},
        {"firebrick", {178, 34, 34}},
        {"floralwhite", {255, 250, 240}},
        {"forestgreen", {34, 139, 34}},
        {"fuchsia", {255, 0, 255}},
        {"gainsboro", {220, 220, 220}},
        {"ghostwhite", {248, 248, 255}},
        {"gold", {255, 215, 0}},
        {"goldenrod", {218, 165, 32}},
        {"gray", {128, 128, 128}},
        {"grey", {128, 128, 128}},
        {"green", {0, 128, 0}},
        {"greenyellow", {173, 255, 47}},
        {"honeydew", {240, 255, 240}},
        {"hotpink", {255, 105, 180}},
        {"indianred", {205, 92, 92}},
        {"indigo", {75, 0, 130}},
        {"ivory", {255, 255, 240}},
        {"khaki", {240, 230, 140}},
        {"lavender", {230, 230, 250}},
        {"lavenderblush", {255, 240, 245}},
        {"lawngreen", {124, 252, 0}},
        {"lemonchiffon", {255, 250, 205}},
        {"lightblue", {173, 216, 230}},
        {"lightcoral", {240, 128, 128}},
        {"lightcyan", {224, 255, 255}},
        {"lightgoldenrodyellow", {250, 250, 210}},
        {"lightgray", {211, 211, 211}},
        {"lightgreen", {144, 238, 144}},
        {"lightgrey", {211, 211, 211}},
        {"lightpink", {255, 182, 193}},
        {"lightsalmon", {255, 160, 122}},
        {"lightseagreen", {32, 178, 170}},
        {"lightskyblue", {135, 206, 250}},
        {"lightslategray", {119, 136, 153}},
        {"lightslategrey", {119, 136, 153}},
        {"lightsteelblue", {176, 196, 222}},
        {"lightyellow", {255, 255, 224}},
        {"lime", {0, 255, 0}},
        {"limegreen", {50, 205, 50}},
        {"linen", {250, 240, 230}},
        {"magenta", {255, 0, 255}},
        {"maroon", {128, 0, 0}},
        {"mediumaquamarine", {102, 205, 170}},
        {"mediumblue", {0, 0, 205}},
        {"mediumorchid", {186, 85, 211}},
        {"mediumpurple", {147, 112, 219}},
        {"mediumseagreen", {60, 179, 113}},
        {"mediumslateblue", {123, 104, 238}},
        {"mediumspringgreen", {0, 250, 154}},
        {"mediumturquoise", {72, 209, 204}},
        {"mediumvioletred", {199, 21, 133}},
        {"midnightblue", {25, 25, 112}},
        {"mintcream", {245, 255, 250}},
        {"mistyrose", {255, 228, 225}},
        {"moccasin", {255, 228, 181}},
        {"navajowhite", {255, 222, 173}},
        {"navy", {0, 0, 128}},
        {"oldlace", {253, 245, 230}},
        {"olive", {128, 128, 0}},
        {"olivedrab", {107, 142, 35}},
        {"orange", {255, 165, 0}},
        {"orangered", {255, 69, 0}},
        {"orchid", {218, 112, 214}},
        {"palegoldenrod", {238, 232, 170}},
        {"palegreen", {152, 251, 152}},
        {"paleturquoise", {175, 238, 238}},
        {"palevioletred", {219, 112, 147}},
        {"papayawhip", {255, 239, 213}},
        {"peachpuff", {255, 218, 185}},
        {"peru", {205, 133, 63}},
        {"pink", {255, 192, 203}},
        {"plum", {221, 160, 221}},
        {"powderblue", {176, 224, 230}},
        {"purple", {128, 0, 128}},
        {"red", {255, 0, 0}},
        {"rosybrown", {188, 143, 143}},
        {"royalblue", {65, 105, 225}},
        {"saddlebrown", {139, 69, 19}},
        {"salmon", {250, 128, 114}},
        {"sandybrown", {244, 164, 96}},
        {"seagreen", {46, 139, 87}},
        {"seashell", {255, 245, 238}},
        {"sienna", {160, 82, 45}},
        {"silver", {192, 192, 192}},
        {"skyblue", {135, 206, 235}},
        {"slateblue", {106, 90, 205}},
        {"slategray", {112, 128, 144}},
        {"slategrey", {112, 128, 144}},
        {"snow", {255, 250, 250}},
        {"springgreen", {0, 255, 127}},
        {"steelblue", {70, 130, 180}},
        {"tan", {210, 180, 140}},
        {"teal", {0, 128, 128}},
        {"thistle", {216, 191, 216}},
        {"tomato", {255, 99, 71}},
        {"turquoise", {64, 224, 208}},
        {"violet", {238, 130, 238}},
        {"wheat", {245, 222, 179}},
        {"white", {255, 255, 255}},
        {"whitesmoke", {245, 245, 245}},
        {"yellow", {255, 255, 0}},
        {"yellowgreen", {154, 205, 50}}
    };
} // sc
