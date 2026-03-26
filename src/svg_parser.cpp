#include <svg_parser.h>
#include <sc.h>
#include "svg_element.h"
#if __cplusplus >= 202002L
typedef ssize_t rsize_t;
#endif
#include <tinyxml2.h>

namespace sc {
    struct svg_parser_impl {
        tinyxml2::XMLDocument doc;

        // static void parse_path_stroke(const std::string &data, std::vector<path_command> &commands) {
        //     color c{data};
        //     commands.emplace(commands.begin(), '<', std::vector<double>{c.cyan(), c.magenta(), c.yellow(), c.black(), c.alpha()});
        // }
        //
        // static void parse_path_fill(const std::string &data, std::vector<path_command> &commands) {
        //     color c{data};
        //     commands.emplace(commands.begin(), '[', std::vector<double>{c.cyan(), c.magenta(), c.yellow(), c.black(), c.alpha()});
        // }

        // static void parse_path_data(const std::string &data, std::vector<path_command> &commands) {
        //     std::istringstream ss(data);
        //     char c;
        //     while (ss >> c) {
        //         std::vector<double> args;
        //         double val;
        //         while (ss.peek() != EOF && !(isalpha(ss.peek()))) {
        //             if (ss.peek() == ' ') {
        //                 ss.get();
        //                 continue;
        //             }
        //             ss >> val;
        //             args.push_back(val);
        //         }
        //         commands.emplace_back(c, args);
        //     }
        // }


        //
        //     static std::vector<path_command> parse_svg(const tinyxml2::XMLElement *element) {
        //         std::vector<path_command> commands;
        //         auto attr = element->FirstAttribute();
        //         while (attr) {
        //             if (std::string(attr->Name()).substr(0, 5) == "xmlns") nullptr;
        //             else if (std::string(attr->Name()) == "width") commands.emplace_back('w', std::vector{std::stod(std::string(attr->Value()))});
        //             else if (std::string(attr->Name()) == "height") commands.emplace_back('h', std::vector{std::stod(std::string(attr->Value()))});
        //             else if (std::string(attr->Name()) == "viewBox") commands.emplace_back('v', string_double_split(std::string(attr->Value())));
        //             else std::cout << "Found svg attr: " << attr->Name() << " " << std::string(attr->Value()).substr(0, 50) << std::endl;
        //             attr = attr->Next();
        //         }
        //         return commands;
        //     }
        //
        //     static std::vector<path_command> parse_rect(const tinyxml2::XMLElement *element) {
        //         std::vector<path_command> commands;
        //         auto attr = element->FirstAttribute();
        //         while (attr) {
        //             if (std::string(attr->Name()) == "width") commands.emplace_back('w', std::vector{std::stod(std::string(attr->Value()))});
        //             else if (std::string(attr->Name()) == "height") commands.emplace_back('h', std::vector{std::stod(std::string(attr->Value()))});
        //             else if (std::string(attr->Name()) == "fill") parse_path_fill(attr->Value(), commands);
        //             else std::cout << "Found rect attr: " << attr->Name() << " " << std::string(attr->Value()).substr(0, 50) << std::endl;
        //             attr = attr->Next();
        //         }
        //         return commands;
        //     }
        //
        //     static std::vector<path_command> parse_path(const tinyxml2::XMLElement *element) {
        //         std::vector<path_command> commands;
        //         auto attr = element->FirstAttribute();
        //         while (attr) {
        //             if (std::string(attr->Name()) == "d") parse_path_data(attr->Value(), commands);
        //             else if (std::string(attr->Name()) == "stroke") parse_path_stroke(attr->Value(), commands);
        //             else if (std::string(attr->Name()) == "fill") parse_path_fill(attr->Value(), commands);
        //             else std::cout << "Found path attr: " << attr->Name() << " " << std::string(attr->Value()).substr(0, 50) << std::endl;
        //             attr = attr->Next();
        //         }
        //         return commands;
        //     }
    };

    std::map<std::string, std::string> attributes(const tinyxml2::XMLElement *element) {
        std::map<std::string, std::string> attributes;
        auto attr = element->FirstAttribute();
        while (attr) {
            attributes[attr->Name()] = attr->Value();
            //     if (std::string(attr->Name()) == "d") parse_path_data(attr->Value(), commands);
            //     else if (std::string(attr->Name()) == "stroke") parse_path_stroke(attr->Value(), commands);
            //     else if (std::string(attr->Name()) == "fill") parse_path_fill(attr->Value(), commands);
            //     else std::cout << "Found path attr: " << attr->Name() << " " << std::string(attr->Value()).substr(0, 50) << std::endl;
            attr = attr->Next();
        }
        return attributes;
    }

    void walk(tinyxml2::XMLElement *element, svg_processor *container, svg_element *parent = nullptr) {
        if (!element) return;
        if (!parent) parent = container->root();
        const auto svg = svg_element::generator(parent, element->Name(), attributes(element));
        // if (name == "g") std::cout << "Found group" << std::endl;
        // else if (name == "svg") {
        //     auto cmds = svg_parser_impl::parse_svg(element);
        //     container->process_header(cmds);
        // } else if (name == "path") {
        //     // auto cmds = svg_parser_impl::parse_path(element);
        //     // if (!cmds.empty()) container->process_path(cmds);
        // } else if (name == "rect") {
        //     // auto cmds = svg_parser_impl::parse_rect(element);
        //     // path_func(cmds);
        // } else std::cerr << "Unhandled element: " << name << std::endl;

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
    }
}
