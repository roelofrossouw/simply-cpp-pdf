#include "../include/pdf_rect.h"

namespace sc {
    pdf_rect::pdf_rect(const rect &r, const double page_height) {
        LowerLeftX = r.left();
        LowerLeftY = page_height - r.bottom();
        UpperRightX = r.right();
        UpperRightY = page_height - r.top();
    }

    double pdf_rect::top() const {
        return UpperRightY;
    }

    double pdf_rect::bottom() const {
        return LowerLeftY;
    }

    double pdf_rect::left() const {
        return LowerLeftX;
    }

    double pdf_rect::right() const {
        return UpperRightX;
    }

    double pdf_rect::width() const {
        return UpperRightX - LowerLeftX;
    }

    double pdf_rect::height() const {
        return UpperRightY - LowerLeftY;
    }

    double pdf_rect::center() const {
        return (LowerLeftX + UpperRightX) / 2;
    }

    double pdf_rect::middle() const {
        return (LowerLeftY + UpperRightY) / 2;
    }
}
