#include "font_collection.h"

#include <PDFWriter.h>
#include <PDFUsedFont.h>
#include <font.h>

#include <stdexcept>

namespace sc {
    font_collection::font_collection(PDFWriter &writer) : writer(writer) {
    }

    PDFUsedFont *font_collection::get(const font &spec) {
        const auto key = std::tuple{spec.family, spec.weight, spec.italic, spec.style};
        if (const auto found = fonts.find(key); found != fonts.end()) return found->second;
        const auto face = spec.face();
        if (!face) throw std::runtime_error("sc::pdf: fontconfig found no font for \"" + spec.family + "\"");
        const auto &file = face->file();
        PDFUsedFont *font = writer.GetFontForFile(file, face->index());
        if (!font) throw std::runtime_error("sc::pdf: can't use the font file " + file);
        fonts.emplace(key, font);
        return font;
    }
}
