#include "../include/font_collection.h"

#include <iostream>
#include <fontconfig/fontconfig.h>

namespace sc {
    font_collection::font_collection() {
        if (!FcInit()) {
            invalid = true;
        }
    }

    font_collection::~font_collection() {
        FcFini();
    }


    const std::string &font_collection::get_font_file(const font_spec &spec) {
        if (font_files.contains(spec)) return font_files[spec];
        std::string FontFile;

        if (invalid) return std::move(*new std::string);

        // FcPattern *pat = FcNameParse(reinterpret_cast<const FcChar8 *>(font_name.c_str()));
        FcPattern *pat = FcPatternCreate();
        FcPatternAddString(pat, FC_FAMILY, spec.fc_family());
        if (spec.fc_weight() != FC_WEIGHT_NORMAL) FcPatternAddInteger(pat, FC_WEIGHT, spec.fc_weight());
        if (spec.is_italic()) FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ITALIC);
        if (spec.has_style()) FcPatternAddString(pat, FC_STYLE, spec.fc_style());

        FcConfigSubstitute(nullptr, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);

        FcResult result;
        FcPattern *font = FcFontMatch(nullptr, pat, &result);
        if (font) {
            FcChar8 *dig = nullptr;
            FcPatternGetString(font, FC_FULLNAME, 0, &dig);
            FcChar8 *file = nullptr;
            if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
                FontFile = reinterpret_cast<const char *>(file);
            }
            FcPatternDestroy(font);
        }
        FcPatternDestroy(pat);

        font_files[spec] = std::move(FontFile);
        return font_files[spec];
    }

    void font_collection::add_font(const font_spec &font, PDFUsedFont *font_ptr) {
        font_refs[font] = font_ptr;
    }

    PDFUsedFont *font_collection::get_font(const font_spec &font) const {
        return font_refs.contains(font) ? font_refs.at(font) : nullptr;
    }
} // rjr
