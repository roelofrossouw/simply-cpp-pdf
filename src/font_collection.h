#ifndef SC_FONT_COLLECTION_H
#define SC_FONT_COLLECTION_H

#include <font.h>

#include <tuple>

#include <map>
#include <string>

class PDFUsedFont;
class PDFWriter;

namespace sc {
    // The fonts one PDF uses, each found once (sc::font_face, through fontconfig) and loaded into the
    // file once.
    class font_collection {
    public:
        explicit font_collection(PDFWriter &writer);

        // Throws std::runtime_error when fontconfig finds no font file, or PDFHummus can't load it.
        PDFUsedFont *get(const font &spec);

    private:
        PDFWriter &writer;
        // One PDFUsedFont serves every size of a face: keyed by family, weight, italic and style.
        std::map<std::tuple<std::string, int, bool, std::string>, PDFUsedFont *> fonts;
    };
}

#endif //SC_FONT_COLLECTION_H
