#ifndef SC_FONT_COLLECTION_H
#define SC_FONT_COLLECTION_H

#include "font_spec.h"
#include <map>

class PDFUsedFont;

namespace sc {
    class font_collection {
    public:
        font_collection();

        ~font_collection();

        const std::string &get_font_file(const font_spec &spec);

        void add_font(const font_spec &font, PDFUsedFont *font_ptr);

        [[nodiscard]] PDFUsedFont *get_font(const font_spec &font) const;

    private:
        std::map<font_spec, std::string> font_files;
        std::map<font_spec, PDFUsedFont *> font_refs;
        bool invalid;
    };
} // sc

#endif //SC_FONT_COLLECTION_H
