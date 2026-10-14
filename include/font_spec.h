#ifndef SC_FONT_SPEC_H
#define SC_FONT_SPEC_H

#include <compare>
#include <string>

namespace sc {
    // A font as fontconfig looks it up: a family ("DejaVu Sans", "Roboto", or a generic name such as
    // "sans-serif" or "serif"), a CSS weight from 100 (thin) to 900 (black) with 400 regular and 700
    // bold, italic or not, and optionally a style name ("Condensed Bold") that wins over the rest.
    // fontconfig picks the closest installed font, so a missing family still gives a font.
    struct font_spec {
        std::string family;
        int weight;
        bool italic;
        std::string style;

        font_spec(std::string family = "sans-serif", int weight = 400, bool italic = false, std::string style = {});

        static font_spec bold(std::string family = "sans-serif") { return {std::move(family), 700}; }

        auto operator<=>(const font_spec &) const = default;
    };
}

#endif //SC_FONT_SPEC_H
