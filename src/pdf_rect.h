#ifndef SC_PDF_RECT_H
#define SC_PDF_RECT_H

#include <PDFRectangle.h>
#include <rect.h>

namespace sc {
    // An sc::rect (y down from the top of the page) in PDF coordinates (y up from the bottom).
    class pdf_rect : public PDFRectangle {
    public:
        pdf_rect(const rect &r, double page_height);

        [[nodiscard]] double top() const { return UpperRightY; }

        [[nodiscard]] double bottom() const { return LowerLeftY; }

        [[nodiscard]] double left() const { return LowerLeftX; }

        [[nodiscard]] double right() const { return UpperRightX; }

        [[nodiscard]] double width() const { return UpperRightX - LowerLeftX; }

        [[nodiscard]] double height() const { return UpperRightY - LowerLeftY; }
    };
}

#endif //SC_PDF_RECT_H
