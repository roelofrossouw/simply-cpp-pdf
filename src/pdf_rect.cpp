#include "pdf_rect.h"

namespace sc {
    pdf_rect::pdf_rect(const rect &r, const double page_height)
        : PDFRectangle(r.left(), page_height - r.bottom(), r.right(), page_height - r.top()) {
    }
}
