#include "font_spec.h"

#include <utility>

namespace sc {
    font_spec::font_spec(std::string family, const int weight, const bool italic, std::string style)
        : family(std::move(family)), weight(weight), italic(italic), style(std::move(style)) {
    }
}
