#include "svg_element.h"
#include <iostream>
#include <math.h>
#include <sstream>

namespace sc {
    std::ostream &operator<<(std::ostream &lhs, const svg_point &rhs) {
        lhs << "(" << rhs.x << "," << rhs.y << ")";
        return lhs;
    }

    std::ostream &operator<<(std::ostream &lhs, const rect &rhs) {
        lhs << "rect(" << rhs.left() << "," << rhs.top() << ") @ (" << rhs.width() << "x" << rhs.height() << ")";
        return lhs;
    }

    std::ostream &operator<<(std::ostream &lhs, const svg_element &rhs) {
        lhs << rhs.string() << std::endl;
        for (auto const &child: rhs.children) lhs << *child;
        return lhs;
    }

    std::vector<double> string_split_to_double(const std::string &str, const char delim = ' ') {
        std::vector<double> tokens;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delim)) {
            tokens.push_back(std::stod(item));
        }
        return tokens;
    }

    svg_point &svg_point::operator+=(const svg_point &add) {
        x += add.x;
        y += add.y;
        return *this;
    }

    svg_point svg_point::operator+(const svg_point &add) const {
        return svg_point{x + add.x, y + add.y};
    }

    svg_point svg_point::operator-(const svg_point &sub) const {
        return svg_point{x - sub.x, y - sub.y};
    }

    void svg_point::rotate(double angle, svg_point center) {
        double angle_radians = angle * M_PI / 180.0;
        double cos_a = cos(angle_radians);
        double sin_a = sin(angle_radians);
        auto norm = *this - center;
        x = cos_a * (norm.x) - sin_a * (norm.y) + center.x;
        y = sin_a * (norm.x) + cos_a * (norm.y) + center.y;
    }

    svg_point operator*(double lhs, const svg_point &rhs) {
        return svg_point{lhs * rhs.x, lhs * rhs.y};
    }

    svg_element::svg_element(bool relative) : relative(relative) {
        points.resize(2);
    }

    svg_element::svg_element(const svg_element &other) {
        relative = other.relative;
        fill = other.fill;
        stroke_color = other.stroke_color;
        stroke_width = other.stroke_width;
        svg = other.svg;
        root = other.root;
        for (auto p: other.points) points.push_back(p);
        for (auto c: other.children) add_child(c->clone(), false);
    }

    svg_element::svg_element(const svg_element &other, svg_element *parent) : svg_element(other) {
        parent->add_child(this, false);
        svg = parent->get_svg();
    }

    svg_element::~svg_element() {
        for (const auto &child: children) {
            delete child;
        }
    }

    svg_element *svg_element::add_child(svg_element *child, bool is_svg) {
        auto last_child = children.empty() ? nullptr : children.back();
        children.push_back(child);
        child->parent = this;
        child->root = root;
        if (is_svg) svg = child;
        if (svg) child->svg = svg;
        child->set_start(last_child ? last_child->get_end() : get_start());
        if (last_child) child->previous = last_child;
        return child;
    }

    svg_element *svg_element::add_child(svg_element *child, const std::map<std::string, std::string> &attributes, bool is_svg = false) {
        add_child(child, is_svg);
        child->add_attributes(attributes);
        return child;
    }

    svg_element *svg_element::add_child(svg_element *child, const std::vector<double> &values) {
        add_child(child);
        child->add_attributes(values);
        return child;
    }

    std::string svg_element::path() {
        std::stringstream ss;
        if (parent) ss << parent->path() << " ";
        ss << type();
        return ss.str();
    }

    std::string svg_element::string() const {
        std::stringstream ss;
        if (parent) ss << parent->path() << " ";
        ss << type();
        for (auto const &point: points) ss << " " << point;
        return ss.str();
    }

    svg_point svg_element::get_point(int i) const { return i < 0 || i >= points.size() ? svg_point{0, 0} : points[i]; }
    void svg_element::set_point(int i, double x, double y, bool rel) { set_point(i, {x, y}, rel); }

    void svg_element::set_point(int i, const svg_point pt, const bool rel) {
        if (i > points.size() - 1) points.resize(i + 1);
        points[i] = rel ? pt + get_start() : pt;
    }

    svg_element *svg_element::generator(svg_element *parent, const std::string &name, const std::map<std::string, std::string> &attributes) {
        if (name == "svg") return parent->add_child(new svg_header(), attributes, true);
        if (name == "path") return parent->add_child(new svg_path(), attributes);
        if (name == "clipPath") return parent->add_child(new svg_path(), attributes);
        if (name == "rect") return parent->add_child(new svg_rect(), attributes);
        if (name == "use") return parent->add_child(new svg_use(), attributes);
        if (name == "g") {
            auto group = parent->add_child(new svg_group(), attributes);
            for (auto &child: group->children) {
                child->add_attributes(attributes);
            }
            return group;
        }
        std::cerr << "Unhandled element type: " << name << std::endl;
        return nullptr;
    }

    void svg_element::set_fill(const std::string &color_spec) {
        fill = color::from_string(color_spec);
    }

    svg_point svg_element::get_start() const { return get_point(0); }
    svg_point svg_element::get_end() const { return get_point(1); }
    void svg_element::set_start(const double x, const double y) { set_start(svg_point{x, y}); }
    void svg_element::set_start_x(const double x) { set_point(0, svg_point{x, get_point(0).y}); }
    void svg_element::set_start_y(const double y) { set_point(0, svg_point{get_point(0).x, y}); }
    void svg_element::set_start(const svg_point &pt) { set_point(0, pt); }
    void svg_element::set_end(const double x, const double y) { set_end(svg_point{x, y}); }

    void svg_element::set_end(const svg_point &pt) {
        set_point(1, pt);
    }

    void svg_element::set_relative_end(double x, double y) {
        svg_point endpoint{x, y};
        set_relative_end(endpoint);
    }

    void svg_element::set_relative_end(svg_point &endpoint) {
        if (relative) endpoint += get_start();
        set_end(endpoint);
    }

    void svg_element::add_attributes(const std::map<std::string, std::string> &attributes) {
        for (const auto &[attr, value]: attributes) add_attribute(attr, value);
    }

    void svg_element::add_attributes(const std::vector<double> &attributes) {
    }

    void svg_element::add_attribute(const std::string &name, const std::string &value) {
    }

    std::string svg_element::type() const {
        return "Unknown";
    }

    std::vector<double> svg_element::parse_transform_params(const std::string &s) {
        std::vector<double> params;
        std::stringstream ss;
        for (char ch: s) {
            if (ch == ',' || std::isspace(static_cast<unsigned char>(ch))) ch = ' ';
            ss << ch;
        }
        double val;
        while (ss >> val) {
            params.push_back(val);
        }
        return params;
    }

    void svg_element::parse_transform(const std::string &transform) {
        // std::cout << "Transform " << type() << ": " << transform << std::endl;
        std::stringstream ss(transform);
        std::string token;
        while (std::getline(ss, token, ')')) {
            if (token.empty()) continue;
            auto pos = token.find('(');
            if (pos == std::string::npos) continue;

            std::string name = token.substr(0, pos);
            std::string args = token.substr(pos + 1);
            auto params = parse_transform_params(args);

            if (name.find("translate") != std::string::npos) {
                double tx = params.size() > 0 ? params[0] : 0;
                double ty = params.size() > 1 ? params[1] : 0;
                normalize({tx, ty});
            } else if (name.find("rotate") != std::string::npos) {
                double angle = params.size() > 0 ? params[0] : 0;
                if (params.size() > 2) rotate(angle, {params[1], params[2]});
                else rotate(angle);
            } else if (name.find("scale") != std::string::npos) {
                double sx = params.size() > 0 ? params[0] : 1;
                double sy = params.size() > 1 ? params[1] : sx;
                scale(sx, sy);
                // } else if (name.find("skewX") != std::string::npos) {
                //     skewX(params.size() > 0 ? params[0] : 0);
                // } else if (name.find("skewY") != std::string::npos) {
                //     skewY(params.size() > 0 ? params[0] : 0);
                // } else if (name.find("matrix") != std::string::npos && params.size() == 6) {
                //     matrix(params[0], params[1], params[2], params[3], params[4], params[5]);
            }
        }
    }

    void svg_xml::add_attribute(const std::string &name, const std::string &value) {
        add_common_attribute(name, value);
    }

    std::string svg_xml::type() const {
        return "XML";
    }

    svg_xml::svg_xml() {
        root = this;
    }

    svg_element *svg_xml::clone() const {
        return new svg_xml(*this);
    }

    void svg_header::set_width(const double w) {
        viewBox = rect(viewBox.left(), viewBox.top(), abs(w), viewBox.height());
    }

    void svg_header::set_height(const double h) {
        viewBox = rect(viewBox.left(), viewBox.top(), viewBox.width(), abs(h));
    }

    void svg_header::set_viewBox(const std::vector<double> &values) {
        set_viewBox(values[0], values[1], values[2], values[3]);
    }

    void svg_header::set_viewBox(const double x, const double y, const double w, const double h) {
        viewBox = rect(x, y, w, h);
        //set_width(viewBox.width());
        // set_height(viewBox.height());
        // if (width == 0) set_width(viewBox.width());
        // if (height == 0) set_height( viewBox.height());
    }

    void svg_header::add_named_element(svg_element *element) {
        named_elements[element->get_id()] = element;
    }

    svg_element *svg_header::get_named_element(const std::string &name) {
        if (!named_elements.contains(name)) return nullptr;
        return named_elements[name];
    }

    svg_element *svg_header::clone() const {
        return new svg_header(*this);
    }

    void svg_header::add_attribute(const std::string &name, const std::string &value) {
        if (name.substr(0, 5) == "xmlns") return;
        if (name == "width") set_width(string_split_to_double(value)[0]);
        else if (name == "height") set_height(string_split_to_double(value)[0]);
        else if (name == "viewBox") set_viewBox(string_split_to_double(value));
        else std::cout << "Unknown header attribute: " << name << " = " << value << std::endl;
    }


    std::string svg_header::type() const {
        return "SVG";
    }

    std::string svg_header::string() const {
        std::stringstream ss;
        ss << svg_element::string() << " viewBox=" << viewBox;
        return ss.str();
    }

    void svg_group::add_attribute(const std::string &name, const std::string &value) {
        if (add_common_attribute(name, value)) return;
        else std::cout << "Unknown group attribute: " << name << " = " << value << std::endl;
    }

    std::string svg_group::type() const {
        return "Group";
    }

    std::string svg_group::string() const {
        std::stringstream ss;
        ss << svg_element::string() << " ID=" << id;
        return ss.str();
    }

    void svg_group::flip() {
        std::cout << "Flipping group " << id << ", " << children[0]->get_svg() << "s " << type();
        std::cout << " before " << children[0]->get_start();
        svg_element::flip();
        std::cout << " after " << children[0]->get_start();
        std::cout << std::endl;
    }

    svg_element *svg_group::clone() const {
        return new svg_group(*this);
    }

    void svg_path::add_command(char type, const std::vector<double> &values) {
        if (type == 'm' || type == 'M') add_child(new svg_move(islower(type)), values);
        else if (type == 'c' || type == 'C') add_child(new svg_cubic_bezier(islower(type)), values);
        else if (type == 'q' || type == 'Q') add_child(new svg_quadratic_curve(islower(type)), values);
        else if (type == 's' || type == 'S') add_child(new svg_smooth_cubic_bezier(islower(type)), values);
        else if (type == 'l' || type == 'L') add_child(new svg_line(islower(type)), values);
        else if (type == 'h' || type == 'H') add_child(new svg_horizontal_line(islower(type)), values);
        else if (type == 'v' || type == 'V') add_child(new svg_vertical_line(islower(type)), values);
        else if (type == 'a' || type == 'A') add_child(new svg_arc(islower(type)), values);
        else if (type == 'z' || type == 'Z') add_child(new svg_close(), values);
        else std::cout << "Unknown command: " << type << " " << values.size() << std::endl;
    }

    void svg_path::parse_path(const std::string &value) {
        std::istringstream ss(value);
        char c;
        while (ss >> c) {
            std::vector<double> args;
            double val;
            while (ss.peek() != EOF && !(isalpha(ss.peek()))) {
                if (ss.peek() == ' ' || ss.peek() == ',' || ss.peek() == '\n' || ss.peek() == '\r' || ss.peek() == '\t') {
                    ss.get();
                    continue;
                }
                ss >> val;
                args.push_back(val);
            }
            add_command(c, args);
        }
    }

    void svg_element::set_stroke(const std::string &color_spec) {
        stroke_color = color::from_string(color_spec);
    }

    void svg_element::set_stroke_width(double new_width) {
        stroke_width = new_width;
    }

    void svg_path::add_attribute(const std::string &name, const std::string &value) {
        if (add_common_attribute(name, value)) return;
        else if (name == "fill") set_fill(value);
        else if (name == "stroke") set_stroke(value);
        else if (name == "stroke-width") set_stroke_width(std::stod(value));
        else if (name == "d") parse_path(value);
        else if (name == "id") id = value;
        else std::cout << "Unknown path attribute: " << name << " = " << value << std::endl;
    }

    std::string svg_path::type() const {
        return "Path";
    }

    std::string svg_path::string() const {
        std::stringstream ss;
        if (fill != color{0, 0, 0, 0}) {
            ss << svg_element::string()
                    << " fill=rgba(" << fill.red() << "," << fill.green() << "," << fill.blue() << "," << fill.alpha()
                    << ")";
        }
        if (stroke_color != color{0, 0, 0, 1}) {
            ss << " stroke=rgba(" << stroke_color.red() << "," << stroke_color.green() << "," << stroke_color.blue() << "," << stroke_color.alpha()
                    << ")";
        }
        if (stroke_width != 1) {
            ss << " stroke-width=" << stroke_width;
        }
        return ss.str();
    }

    svg_element *svg_path::clone() const {
        return new svg_path(*this);
    }

    void svg_clip_path::draw(XObjectContentContext *ctx) const {
        return;
    }

    void svg_move::add_attributes(const std::vector<double> &values) {
        set_relative_end(values[0], values[1]);
        parent->set_start(get_end());
        if (values.size() >= 4) {
            const std::vector<double> rest{values.begin() + 2, values.end()};
            parent->add_child(new svg_line(relative), rest);
        }
    }

    std::string svg_move::type() const {
        return "Move";
    }

    svg_element *svg_move::clone() const {
        return new svg_move(*this);
    }

    void svg_line::add_attributes(const std::vector<double> &values) {
        set_relative_end(values[0], values[1]);
        if (values.size() >= 4) {
            const std::vector<double> rest{values.begin() + 2, values.end()};
            parent->add_child(new svg_line(relative), rest);
        }
    }

    std::string svg_line::type() const {
        return "Line";
    }

    svg_element *svg_line::clone() const {
        return new svg_line(*this);
    }

    void svg_horizontal_line::add_attributes(const std::vector<double> &values) {
        const svg_point endpoint{relative ? values[0] + get_start().x : values[0], get_start().y};
        set_end(endpoint);
    }

    void svg_vertical_line::add_attributes(const std::vector<double> &values) {
        const svg_point endpoint{get_start().x, relative ? values[0] + get_start().y : values[0]};
        set_end(endpoint);
    }

    void svg_arc::add_attributes(const std::vector<double> &values) {
        radius = svg_point{values[0], values[1]};
        x_axis_rotation = values[2];
        large_arc_flag = values[3] > 0.5;
        sweep_flag = values[4] > 0.5;
        set_relative_end(values[5], values[6]);
        if (values.size() >= 14) {
            std::vector<double> rest{values.begin() + 7, values.end()};
            parent->add_child(new svg_arc(relative), rest);
        }
    }

    std::string svg_arc::type() const {
        return "Arc";
    }

    std::string svg_arc::string() const {
        std::stringstream ss;
        ss << svg_element::string()
                << " radius=" << radius
                << " x_axis_rotation=" << x_axis_rotation
                << " large_arc_flag=" << (large_arc_flag ? "Yes" : "No")
                << " sweep_flag=" << (sweep_flag ? "Yes" : "No");
        return ss.str();
    }

    void svg_arc::draw(XObjectContentContext *ctx) const {
        // ctx->l(get_end().x, get_end().y);

        auto x1 = get_start().x;
        auto y1 = get_start().y;
        auto x2 = get_end().x;
        auto y2 = get_end().y;
        auto rx = radius.x;
        auto ry = radius.y;
        auto angle = x_axis_rotation;


        // Convert angle to radians
        double phi = angle * M_PI / 180.0;
        double cosPhi = std::cos(phi);
        double sinPhi = std::sin(phi);

        // Step 1: Compute transformed coordinates
        double dx = (x1 - x2) / 2.0;
        double dy = (y1 - y2) / 2.0;
        double x1p = cosPhi * dx + sinPhi * dy;
        double y1p = -sinPhi * dx + cosPhi * dy;

        // Correct radii
        double rx2 = rx * rx;
        double ry2 = ry * ry;
        double x1p2 = x1p * x1p;
        double y1p2 = y1p * y1p;
        double lambda = x1p2 / rx2 + y1p2 / ry2;
        if (lambda > 1.0) {
            double scale = std::sqrt(lambda);
            rx *= scale;
            ry *= scale;
            rx2 = rx * rx;
            ry2 = ry * ry;
        }

        // Step 2: Compute center
        double sign = (large_arc_flag == sweep_flag) ? -1.0 : 1.0;
        double num = rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2;
        double den = rx2 * y1p2 + ry2 * x1p2;
        double factor = sign * std::sqrt(std::max(0.0, num / den));
        double cxp = factor * (rx * y1p / ry);
        double cyp = factor * (-ry * x1p / rx);

        double cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2.0;
        double cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2.0;

        // Step 3: Compute angles
        auto angleBetween = [](double ux, double uy, double vx, double vy) {
            double dot = ux * vx + uy * vy;
            double len = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
            double ang = std::acos(std::max(-1.0, std::min(1.0, dot / len)));
            if (ux * vy - uy * vx < 0) ang = -ang;
            return ang;
        };

        double theta1 = angleBetween(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);
        double deltaTheta = angleBetween((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);

        if (!sweep_flag && deltaTheta > 0) deltaTheta -= 2 * M_PI;
        else if (sweep_flag && deltaTheta < 0) deltaTheta += 2 * M_PI;

        // Step 4: Split arc into segments of max 90°
        int segments = static_cast<int>(std::ceil(std::fabs(deltaTheta / (M_PI / 2.0))));
        double delta = deltaTheta / segments;

        for (int i = 0; i < segments; ++i) {
            double t1 = theta1 + i * delta;
            double t2 = t1 + delta;

            // Endpoints
            double cosT1 = std::cos(t1), sinT1 = std::sin(t1);
            double cosT2 = std::cos(t2), sinT2 = std::sin(t2);

            std::pair<double, double> P0 = {
                cx + rx * cosPhi * cosT1 - ry * sinPhi * sinT1,
                cy + rx * sinPhi * cosT1 + ry * cosPhi * sinT1
            };
            std::pair<double, double> P3 = {
                cx + rx * cosPhi * cosT2 - ry * sinPhi * sinT2,
                cy + rx * sinPhi * cosT2 + ry * cosPhi * sinT2
            };

            // Derivatives
            double alpha = std::tan(delta / 4.0);
            double factor = (4.0 / 3.0) * alpha / (1 + alpha * alpha);

            std::pair<double, double> P1 = {
                P0.first - factor * (rx * cosPhi * sinT1 + ry * sinPhi * cosT1),
                P0.second - factor * (rx * sinPhi * sinT1 - ry * cosPhi * cosT1)
            };
            std::pair<double, double> P2 = {
                P3.first + factor * (rx * cosPhi * sinT2 + ry * sinPhi * cosT2),
                P3.second + factor * (rx * sinPhi * sinT2 - ry * cosPhi * cosT2)
            };

            ctx->c(P1.first, P1.second, P2.first, P2.second, P3.first, P3.second);
            //   curves.push_back({P0,P1,P2,P3});
        }
    }

    void svg_arc::scale(double scale_x, double scale_y) {
        radius.x *= scale_x;
        radius.y *= scale_y;
        svg_element::scale(scale_x, scale_y);
    }

    svg_element *svg_arc::clone() const {
        return new svg_arc(*this);
    }

    void svg_close::add_attributes(const std::vector<double> &values) {
        set_end(parent->get_start());
    }

    std::string svg_close::type() const {
        return "Close";
    }


    void svg_cubic_bezier::add_attributes(const std::vector<double> &values) {
        set_point(2, values[0], values[1], relative);
        set_point(3, values[2], values[3], relative);
        set_relative_end(values[4], values[5]);
        if (values.size() >= 12) {
            std::vector<double> rest{values.begin() + 6, values.end()};
            parent->add_child(new svg_cubic_bezier(relative), rest);
        }
    }

    std::string svg_cubic_bezier::type() const {
        return "Cubic Bezier";
    }

    svg_element *svg_quadratic_curve::clone() const {
        return new svg_quadratic_curve(*this);
    }

    void svg_smooth_cubic_bezier::add_attributes(const std::vector<double> &values) {
        set_point(2, previous->get_point(3));
        set_point(3, values[0], values[1], relative);
        set_relative_end(values[2], values[3]);
        if (values.size() >= 8) {
            std::vector<double> rest{values.begin() + 4, values.end()};
            parent->add_child(new svg_smooth_cubic_bezier(relative), rest);
        }
    }


    void svg_rect::add_attributes(const std::map<std::string, std::string> &attributes) {
        svg_element::add_attributes(attributes);
        set_point(2, width, height);
    }

    void svg_rect::add_attribute(const std::string &name, const std::string &value) {
        if (add_common_attribute(name, value)) return;
        else if (name == "width") width = std::stod(value);
        else if (name == "height") height = std::stod(value);
        else if (name == "rx") radius_x = std::stod(value);
        else if (name == "ry") radius_y = std::stod(value);
        else std::cout << "Unknown rect attribute: " << name << " = " << value << std::endl;
    }


    std::string svg_rect::string() const {
        std::stringstream ss;
        ss << svg_element::string() << " fill=rgba(" << fill.red() << "," << fill.green() << "," << fill.blue() << "," << fill.alpha() << ")";
        if (radius_x > 0 || radius_y > 0) ss << " radius=(" << radius_x << "," << radius_y << ")";
        return ss.str();
    }

    std::string svg_rect::type() const {
        return "Rect";
    }

    void svg_rect::scale(double scale_x, double scale_y) {
        width *= scale_x;
        height *= scale_y;
        svg_element::scale(scale_x, scale_y);
    }

    void svg_rect::flip() {
        height = -height;
        svg_element::flip();
    }

    void svg_header::scale(double scale_x, double scale_y) {
        set_width(width() * scale_x);
        set_height(height() * scale_y);
        svg_element::scale(scale_x, scale_y);
    }

    void svg_use::add_attribute(const std::string &name, const std::string &value) {
        if (add_common_attribute(name, value)) return;
        else if (name == "clip-path") return;
        else if (name == "xlink:href") fetch_named_element(value);
        else std::cout << "Unknown use attribute: " << name << " = " << value << std::endl;
    }

    std::string svg_use::type() const {
        return "Use";
    }

    std::string svg_use::string() const {
        std::stringstream ss;
        ss << svg_element::string();
        return ss.str();
    }

    svg_element *svg_use::clone() const {
        return new svg_use(*this);
    }

    void svg_element::draw(XObjectContentContext *ctx) const {
        if (fill != color{0, 0, 0, 0}) ctx->rg(fill.red(), fill.green(), fill.blue());
        for (auto const &child: children) { child->draw(ctx); }
    }

    void svg_element::scale(double scale_x, double scale_y) {
        for (auto &p: points) {
            p.x *= scale_x;
            p.y *= scale_y;
            // if (svg && (scale_x < 0 || scale_y < 0)) {
            //     // std::cout << "Negative scale: " << scale_x << " x " << scale_y << " dims: " << svg->height() << "x" << svg->height() << " Old point " << p;
            //     if (scale_x < 0) p.x += svg->width(); // - p.x;
            //     if (scale_y < 0) p.y += svg->height(); // - p.y;
            //     // std::cout << " Nwd point " << p << std::endl;
            // }
        }
        for (auto &child: children) { child->scale(scale_x, scale_y); }
    }

    double svg_element::width() const {
        if (type() == "XML" && !children.empty()) return children[0]->width();
        if (parent && parent->type() != "XML") return parent->width();
        return 100;
    }

    double svg_element::height() const {
        if (type() == "XML" && !children.empty()) return children[0]->height();
        if (parent && parent->type() != "XML") return parent->height();
        return 100;
    }

    rect svg_element::get_viewBox() const {
        if (type() == "XML" && !children.empty()) return children[0]->get_viewBox();
        if (parent && parent->type() != "XML") return parent->get_viewBox();
        return {0, 0};
    }

    void svg_element::set_id(const std::string &value) {
        id = value;
        reinterpret_cast<svg_header *>(svg)->add_named_element(this);
    }

    double svg_header::width() const {
        return viewBox.width();
    }

    double svg_header::height() const {
        return viewBox.height();
    }

    rect svg_header::get_viewBox() const {
        return viewBox;
    }

    bool svg_element::add_common_attribute(const std::string &name, const std::string &value) {
        if (name == "id") set_id(value);
        else if (name == "fill") set_fill(value);
        else if (name == "stroke") set_stroke(value);
        else if (name == "stroke-width") set_stroke_width(std::stod(value));
        else if (name == "transform") transform = value;
        else if (name == "clip-path") return true;
        else if (name == "x") set_start_x(std::stod(value));
        else if (name == "y") set_start_y(std::stod(value));
        else return false;
        return true;
    }

    void svg_element::resize(int new_width, int new_height, bool keep_aspect) {
        double scale_x = new_width / width();
        double scale_y = new_height / height();
        if (keep_aspect) {
            scale_x = scale_y = std::min(scale_x, scale_y);
        }
        scale(scale_x, scale_y);
    }

    void svg_element::fetch_named_element(const std::string &name) {
        const auto element = reinterpret_cast<svg_header *>(svg)->get_named_element(name.substr(1));
        if (!element) return; // Named element not found :(
        add_child(element->clone(), false);
    }

    void svg_element::rotate(double angle, svg_point center) {
        // std::cout << "Rotate " << type() << " around " << center << std::endl;
        for (auto &p: points) p.rotate(angle, center);
        for (auto &child: children) child->rotate(angle, center);
    }

    void svg_element::normalize() {
        auto vb = get_viewBox();
        svg_point mv{-vb.left(), -vb.top()};
        normalize(mv);
    }

    void svg_element::normalize(svg_point mv) {
        for (auto &p: points) p += mv;
        for (auto &child: children) child->normalize(mv);
    }

    std::string svg_element::get_id() {
        return id;
    }

    void svg_element::do_transform() {
        if (!transform.empty()) {
            std::cout << "Transform " << type() << ": " << transform << std::endl;
            parse_transform(transform);
            transform = ""; // Prevent double transform
        }
        for (auto &child: children) child->do_transform();
    }

    void svg_element::flip() {
        if (svg) for (auto &p: points) p.y = svg->height() - p.y;
        for (auto &child: children) { child->flip(); }
    }

    svg_element *svg_element::get_svg() const {
        return svg;
    }

    void svg_path::draw(XObjectContentContext *ctx) const {
        if (fill.alpha() != 0) {
            ctx->rg(fill.red(), fill.green(), fill.blue());
            for (auto const &child: children) { child->draw(ctx); }
            ctx->f();
        }
        if (stroke_color.alpha() != 0) {
            ctx->RG(0, 0, 1); // stroke_color.red(), stroke_color.green(), stroke_color.blue());
            ctx->w(0.1); // stroke_width);
            for (auto const &child: children) { child->draw(ctx); }
            ctx->s();
        }
    }

    std::string svg_clip_path::type() const {
        return "ClipPath";
    }

    void svg_move::draw(XObjectContentContext *ctx) const {
        ctx->m(get_end().x, get_end().y);
    }

    void svg_line::draw(XObjectContentContext *ctx) const {
        ctx->l(get_end().x, get_end().y);
    }

    void svg_close::draw(XObjectContentContext *ctx) const {
        ctx->h();
    }

    svg_element *svg_close::clone() const {
        return new svg_close(*this);
    }

    void svg_cubic_bezier::draw(XObjectContentContext *ctx) const {
        ctx->c(points[2].x, points[2].y, points[3].x, points[3].y, points[1].x, points[1].y);
    }

    svg_element *svg_cubic_bezier::clone() const {
        return new svg_cubic_bezier(*this);
    }

    void svg_quadratic_curve::add_attributes(const std::vector<double> &values) {
        set_point(2, values[0], values[1], relative);
        set_relative_end(values[2], values[3]);
        if (values.size() >= 8) {
            std::vector<double> rest{values.begin() + 4, values.end()};
            parent->add_child(new svg_quadratic_curve(relative), rest);
        }
    }

    std::string svg_quadratic_curve::type() const {
        return "QuadraticCurve";
    }

    void svg_quadratic_curve::draw(XObjectContentContext *ctx) const {
        auto c1 = get_start() + ((2 / 3.0) * (get_point(2) - get_start()));
        auto c2 = get_end() + ((2 / 3.0) * (get_point(2) - get_end()));
        ctx->c(c1.x, c1.y, c2.x, c2.y, points[1].x, points[1].y);
    }

    void svg_rect::draw(XObjectContentContext *ctx) const {
        ctx->q();
        if (radius_x <= 0 && radius_y <= 0) {
            ctx->re(get_start().x, get_start().y, width, height);
        } else {
            /*
            if (radius > 0) {
                radius = min(radius, min(rect.width() / 2, rect.height() / 2));
                double control = radius * 0.448216; // Control points needed on cubic bezier curve to form a quarter circle.
                impl->pageContentContext->m(rect.left() + radius, rect.bottom());
                impl->pageContentContext->l(rect.right() - radius, rect.bottom());
                impl->pageContentContext->c(rect.right() - control, rect.bottom(), rect.right(), rect.bottom() + control, rect.right(), rect.bottom() + radius);
                impl->pageContentContext->l(rect.right(), rect.top() - radius);
                impl->pageContentContext->c(rect.right(), rect.top() - control, rect.right() - control, rect.top(), rect.right() - radius, rect.top());
                impl->pageContentContext->l(rect.left() + radius, rect.top());
                impl->pageContentContext->c(rect.left() + control, rect.top(), rect.left(), rect.top() - control, rect.left(), rect.top() - radius);
                impl->pageContentContext->l(rect.left(), rect.bottom() + radius);
                impl->pageContentContext->c(rect.left(), rect.bottom() + control, rect.left() + control, rect.bottom(), rect.left() + radius, rect.bottom());
                impl->pageContentContext->h();
            } else {
                impl->pageContentContext->re(rect.left(), rect.bottom(), rect.width(), rect.height());
            }
            if (filled) impl->pageContentContext->f();
            else impl->pageContentContext->s();
            impl->pageContentContext->Q();

             */
        }
        if (fill.alpha() != 0) {
            ctx->rg(fill.red(), fill.green(), fill.blue());
            ctx->f();
        } else {
            ctx->s();
        }
        // if (stroke.alpha() != 0) ctx->RG(stroke.red(), stroke.green(), stroke.blue());
        ctx->Q();
    }

    svg_element *svg_rect::clone() const {
        return new svg_rect(*this);
    }

    // void svg_smooth_cubic_bezier::draw(XObjectContentContext *ctx) const {
    //     ctx->c(points[2].x, points[2].y, points[3].x, points[3].y, points[1].x, points[1].y);
    // }
} // sc
