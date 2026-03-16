#include <font_spec.h>
#include <iostream>
#include <fontconfig/fontconfig.h>

namespace sc {

    font_spec::font_spec(std::string family, const int weight, const bool italic, std::string style)
        : family(std::move(family)),
          style(std::move(style)),
          weight(weight),
          italic(italic) {
    }

    int font_spec::fc_weight() const {
        if (weight <= 100) return FC_WEIGHT_THIN;
        if (weight <= 200) return FC_WEIGHT_EXTRALIGHT;
        if (weight <= 300) return FC_WEIGHT_LIGHT;
        if (weight <= 350) return FC_WEIGHT_BOOK;
        if (weight <= 400) return FC_WEIGHT_REGULAR;
        if (weight <= 500) return FC_WEIGHT_MEDIUM;
        if (weight <= 600) return FC_WEIGHT_DEMIBOLD;
        if (weight <= 700) return FC_WEIGHT_BOLD;
        if (weight <= 800) return FC_WEIGHT_EXTRABOLD;
        if (weight <= 900) return FC_WEIGHT_BLACK;
        return FC_WEIGHT_REGULAR; // fallback
    }

    const FcChar8 *font_spec::fc_family() const {
        return reinterpret_cast<const FcChar8 *>(family.c_str());
    }

    bool font_spec::operator<(const font_spec &rhs) const {
        return family < rhs.family || (family == rhs.family && weight < rhs.weight)
               || (family == rhs.family && weight == rhs.weight && italic < rhs.italic)
               || (family == rhs.family && weight == rhs.weight && italic == rhs.italic && style < rhs.style);
    }

    bool font_spec::is_italic() const {
        return italic;
    }

    bool font_spec::has_style() const {
        return !style.empty();
    }

    const FcChar8 *font_spec::fc_style() const {
        return reinterpret_cast<const FcChar8 *>(style.c_str());
    }

} // rjr
