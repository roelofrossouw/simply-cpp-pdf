#ifndef SC_PDF_RECT_H
#define SC_PDF_RECT_H

#include "base_types.h"
#include "PDFRectangle.h"

namespace sc {
    class pdf_rect : public PDFRectangle {
    public:
        pdf_rect(const rect &r, double page_height);

        double top() const;

        double bottom() const;

        double left() const;

        double right() const;

        double width() const;

        double height() const;

        double center() const;

        double middle() const;
    };
}

#endif //SC_PDF_RECT_H
