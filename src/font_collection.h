#ifndef SC_FONT_COLLECTION_H
#define SC_FONT_COLLECTION_H

#include "font_spec.h"

#include <map>
#include <string>

class PDFUsedFont;
class PDFWriter;

namespace sc {
    // The fonts one PDF uses, each found through fontconfig once and loaded into the file once.
    class font_collection {
    public:
        explicit font_collection(PDFWriter &writer);

        // Throws std::runtime_error when fontconfig finds no font file, or PDFHummus can't load it.
        PDFUsedFont *get(const font_spec &spec);

        // The font file fontconfig picks for spec, "" when none.
        static std::string font_file(const font_spec &spec);

    private:
        PDFWriter &writer;
        std::map<font_spec, PDFUsedFont *> fonts;
    };
}

#endif //SC_FONT_COLLECTION_H
