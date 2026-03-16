#ifndef TEST_BASE64_FONT_SPEC_H
#define TEST_BASE64_FONT_SPEC_H

#include <string>

namespace sc {

    class font_spec {
        std::string family;
        std::string style;
        int weight;
        bool italic;

    public:
        font_spec(std::string family, int weight = 400, bool italic = false, std::string style = "");

        [[nodiscard]] int fc_weight() const;

        [[nodiscard]] const unsigned char *fc_family() const;

        bool operator<(const font_spec &rhs) const;

        [[nodiscard]] bool is_italic() const;

        [[nodiscard]] bool has_style() const;

        [[nodiscard]] const unsigned char *fc_style() const;
    };



} // rjr

#endif //TEST_BASE64_FONT_SPEC_H
