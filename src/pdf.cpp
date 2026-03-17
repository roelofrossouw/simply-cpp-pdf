#include <pdf.h>
#include <font_collection.h>
#include <pdf_rect.h>
#include <iostream>


typedef size_t rsize_t;

#include "PDFWriter.h"
#include "PDFPage.h"
#include "PageContentContext.h"
#include "PDFFormXObject.h"
#include "PDFUsedFont.h"
#include "ResourcesDictionary.h"
#include "XObjectContentContext.h"


using namespace std;

namespace sc {
    class hummus_impl {
    public:
        PDFWriter *pdfWriter;
        PDFPage *pdfPage;
        PageContentContext *pageContentContext;
        page_size size{};
        vector<pair<PDFPage *, PageContentContext *> > pages;
        double width{};
        double height{};
        unsigned int current_page = 0;

        double page_width() const {
            switch (size) {
                case A4: return 595;
                case A5: return 420;
                case Letter: return 612;
                case Legal: return 612;
                default: return 595;
            }
            return 595;
        }

        double page_height() const {
            switch (size) {
                case A4: return 842;
                case A5: return 594;
                case Letter: return 792;
                case Legal: return 842;
                default: return 842;
            }
            return 842;
        }

        void create_page(page_size new_size = Default) {
            if (pageContentContext) if (eSuccess != pdfWriter->PausePageContentContext(pageContentContext)) cerr << "Error ending Page Page Context";
            if (new_size != Default) size = new_size;
            pdfPage = new PDFPage();
            width = page_width();
            height = page_height();
            pdfPage->SetMediaBox({0, 0, width, height});
            pageContentContext = pdfWriter->StartPageContentContext(pdfPage);
            pages.emplace_back(pdfPage, pageContentContext);
            current_page = pages.size();
        }


        PDFUsedFont *get_font(const font_spec &font_spec) {
            auto font = fc.get_font(font_spec);
            if (font) return font;
            auto font_file = fc.get_font_file(font_spec);
            font = pdfWriter->GetFontForFile(font_file, 0);
            // font->GetFreeTypeFont()->GetFontFlags();
            fc.add_font(font_spec, font);
            return font;
        }

        void SetPage(unsigned int page_number) {
            if (current_page == page_number) return;
            while (pages.size() < page_number) create_page();
            if (pageContentContext) pdfWriter->PausePageContentContext(pageContentContext);
            pdfPage = pages[page_number - 1].first;
            pageContentContext = pages[page_number - 1].second;
            width = page_width();
            height = page_height();
            current_page = page_number;
        }

        hummus_impl(const page_size size = A4) : pdfWriter(new PDFWriter()), pdfPage(nullptr), pageContentContext(nullptr) {
            create_page(size);
        }

        ~hummus_impl() {
            for (auto &page: pages) {
                pdfWriter->EndPageContentContext(page.second);
                pdfWriter->WritePageAndRelease(page.first);
            }
            if (eSuccess != pdfWriter->EndPDF()) cerr << "Error ending PDF";
            delete pdfWriter;
        }

        // private:
        font_collection fc;
    };

    static const string scSystemFontsPath = "/System/Library/Fonts/Supplemental/";

    pdf::pdf(const std::string &filename, page_size size) {
        impl = new hummus_impl(size);
        impl->pdfWriter->StartPDF(filename, ePDFVersion14, LogConfiguration::DefaultLogConfiguration(), PDFCreationSettings(false, true));
    }

    pdf::pdf(const char *filename) {
        impl = new hummus_impl();
        impl->pdfWriter->StartPDF(filename, ePDFVersion14, LogConfiguration::DefaultLogConfiguration(), PDFCreationSettings(false, true));
    }

    pdf::~pdf() {
        // if (eSuccess != impl->pdfWriter->EndPageContentContext(impl->pageContentContext)) cerr << "Error ending Page Page Context";
        // if (eSuccess != impl->pdfWriter->WritePageAndRelease(impl->pdfPage)) cerr << "Error ending PDF Page";
        // if (eSuccess != impl->pdfWriter->EndPDF()) cerr << "Error ending PDF";
        delete impl;
    }

    void pdf::NewPage() {
        impl->create_page();
    }

    bool pdf::DrawImage(const std::string &filename) {
        // Create an image object from image
        PDFFormXObject *image = impl->pdfWriter->CreateFormXObjectFromJPGFile(filename);
        // PDFFormXObject *image = impl->pdfWriter->CreateFormXObjectFromPNGStream();
        if (!image) return false;

        impl->pageContentContext->q();
        impl->pageContentContext->cm(0.4, 0, 0, 0.4, 57.5, 241);
        impl->pageContentContext->Do(impl->pdfPage->GetResourcesDictionary().AddFormXObjectMapping(image->GetObjectID()));
        impl->pageContentContext->Q();

        delete image;
        return true;
    }

    bool pdf::MoreText(const string &text, const rect &pos) {
        const pdf_rect position{pos, impl->height};
        impl->pageContentContext->BT();
        impl->pageContentContext->Td(position.left(), position.bottom());
        impl->pageContentContext->Tj(text);
        impl->pageContentContext->ET();
        return true;
    }

    bool pdf::DrawText(const string &text, const rect &pos, const double &fontsize, const font_spec &font, const color &col) {
        const pdf_rect position{pos, impl->height};
        impl->pageContentContext->BT();
        impl->pageContentContext->k(col.c, col.m, col.y, col.k);
        impl->pageContentContext->Tf(impl->get_font(font), fontsize);
        impl->pageContentContext->Td(position.left(), position.bottom());
        impl->pageContentContext->Tj(text);
        impl->pageContentContext->ET();
        return true;
    }

    void pdf::DrawLine(const rect &pos, double width, const color &col) {
        const pdf_rect rect{pos, impl->height};
        impl->pageContentContext->q();
        impl->pageContentContext->w(width);
        impl->pageContentContext->K(col.c, col.m, col.y, col.k);
        impl->pageContentContext->m(rect.left(), rect.bottom());
        impl->pageContentContext->l(rect.right(), rect.top());
        impl->pageContentContext->s();
        impl->pageContentContext->Q();
    }

    void pdf::DrawRect(const rect &pos, double width, const color &col, double radius, bool filled = false) {
        const pdf_rect rect{pos, impl->height};
        impl->pageContentContext->q();
        if (width > 0) impl->pageContentContext->w(width);
        if (filled) impl->pageContentContext->k(col.c, col.m, col.y, col.k);
        else impl->pageContentContext->K(col.c, col.m, col.y, col.k);
        if (radius > 0) {
            radius = min(radius, min(rect.width() / 2, rect.height() / 2));
            double control = radius * 0.448216; // Control points needed on cubic bezier curve to form a quarter circle.
            impl->pageContentContext->m(rect.left() + radius, rect.bottom());
            impl->pageContentContext->l(rect.right() - radius, rect.bottom());
            impl->pageContentContext->c(rect.right() - control, rect.bottom(), rect.right(), rect.bottom() + control, rect.right(), rect.bottom() + radius);
            impl->pageContentContext->l(rect.right(), rect.top() - radius);
            impl->pageContentContext->c(rect.right(), rect.top() - control, rect.right() - control, rect.top(), rect.right() - radius, rect.top());
            impl->pageContentContext->l(rect.left() + radius, rect.top());
            impl->pageContentContext->c(rect.left() + control, rect.top(), rect.left(), rect.top() - control, rect.left(), rect.top() - radius);
            impl->pageContentContext->l(rect.left(), rect.bottom() + radius);
            impl->pageContentContext->c(rect.left(), rect.bottom() + control, rect.left() + control, rect.bottom(), rect.left() + radius, rect.bottom());
            impl->pageContentContext->h();
        } else {
            impl->pageContentContext->re(rect.left(), rect.bottom(), rect.width(), rect.height());
        }
        if (filled) impl->pageContentContext->f();
        else impl->pageContentContext->s();
        impl->pageContentContext->Q();
    }

    void pdf::StrokeRect(const rect &pos, double width, const color &col, const double radius) {
        DrawRect(pos, width, col, radius, false);
    }

    void pdf::FillRect(const rect &pos, const color &col, const double radius) {
        DrawRect(pos, 0, col, radius, true);
    }

    void pdf::DrawObject() {
        // Pausing content context, to allow form definition
        impl->pdfWriter->PausePageContentContext(impl->pageContentContext);

        // Create a form with a triangle pattern, RGB colorspace, fill
        PDFFormXObject *formXObject = impl->pdfWriter->StartFormXObject(PDFRectangle(0, 0, 400, 400));
        XObjectContentContext *xobjectContent = formXObject->GetContentContext();

        xobjectContent->rg(1, 0, 0);
        xobjectContent->m(0, 0);
        xobjectContent->l(200, 400);
        xobjectContent->l(400, 0);
        xobjectContent->f();

        ObjectIDType formObjectID = formXObject->GetObjectID();
        auto status = impl->pdfWriter->EndFormXObjectAndRelease(formXObject);
        if (status != eSuccess) return;

        // Place the form in multiple locations in the page
        string formNameInPage = impl->pdfPage->GetResourcesDictionary().AddFormXObjectMapping(formObjectID);

        impl->pageContentContext->q();
        impl->pageContentContext->cm(0.5, 0, 0, 0.5, 120, 100);
        impl->pageContentContext->Do(formNameInPage);
        impl->pageContentContext->Q();

        impl->pageContentContext->q();
        impl->pageContentContext->cm(0.2, 0, 0, 0.2, 350, 100);
        impl->pageContentContext->Do(formNameInPage);
        impl->pageContentContext->Q();
    }

    void pdf::AddLink(const std::string &url, const rect &pos) {
        const pdf_rect rect{pos, impl->height};
        auto had_context = impl->pageContentContext != nullptr;
        if (had_context) impl->pdfWriter->EndPageContentContext(impl->pageContentContext);
        impl->pdfWriter->AttachURLLinktoCurrentPage(url, rect);
        if (had_context) impl->pageContentContext = impl->pdfWriter->StartPageContentContext(impl->pdfPage);
    }


    void pdf::Merge() {
        // status = pdfWriter.AppendPDFPagesFromPDF(scBasePath + "XObjectContent.PDF",PDFPageRange()).first;
        // if(status != eSuccess)
        //     break;
        //
        // // appending pages 0,1,2,3,5,6,7,8,9,10 of BasicTIFFImagesTest.PDF
        // PDFPageRange selectivePageRange;
        // selectivePageRange.mType = PDFPageRange::eRangeTypeSpecific;
        // selectivePageRange.mSpecificRanges.push_back(ULongAndULong(0,3));
        // selectivePageRange.mSpecificRanges.push_back(ULongAndULong(5,10));
        //
        // status = pdfWriter.AppendPDFPagesFromPDF(scBasePath + "BasicTIFFImagesTest.PDF",selectivePageRange).first;
    }

    double pdf::PageHeight() const {
        return impl->height;
    }

    double pdf::PageWidth() const {
        return impl->width;
    }

    void pdf::TestEmoji() const {
        auto font = impl->get_font({"Segoe UI Emoji"});
        AbstractContentContext::TextOptions textOptions(font, 48, AbstractContentContext::eGray, 0);
        impl->pageContentContext->WriteText(75, 300, "\xe2\x9a\x93\xe2\x9a\x94\xe2\x98\xba\xf0\x9f\x98\xa8", textOptions);
        impl->pageContentContext->WriteText(75, 350, "😂🤷🤞🤪", textOptions);
    }

    double pdf::TextWidth(const std::string &text, const font_spec &font, const float size) const {
        return impl->get_font(font)->CalculateTextAdvance(text, size);
    }

    double pdf::TextXHeight(const font_spec &font) const {
        auto result = impl->get_font(font)->GetFreeTypeFont()->GetxHeight();
        if (result.first) return result.second;
        return 0;
    }

    double pdf::TextCapHeight(const font_spec &font) const {
        auto result = impl->get_font(font)->GetFreeTypeFont()->GetCapHeight();
        if (result.first) return result.second;
        return 0;
    }

    void pdf::SetPage(unsigned int page_number) {
        impl->SetPage(page_number);
    }

    unsigned int pdf::PageCount() const {
        return impl->pages.size();
    }
}
