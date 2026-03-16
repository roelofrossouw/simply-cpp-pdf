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
        page_size size;
        double width;
        double height;

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
            if (pageContentContext)
                if (eSuccess != pdfWriter->EndPageContentContext(pageContentContext)) cerr << "Error ending Page Page Context";
            if (pdfPage)
                if (eSuccess != pdfWriter->WritePageAndRelease(pdfPage)) cerr << "Error ending PDF Page";

            if (new_size != Default) size = new_size;

            pdfPage = new PDFPage();
            width = page_width();
            height = page_height();
            pdfPage->SetMediaBox({0, 0, width, height});
            pageContentContext = pdfWriter->StartPageContentContext(pdfPage);
        }


        PDFUsedFont *get_font(const font_spec &font_spec) {
            auto font = fc.get_font(font_spec);
            if (font) return font;
            auto font_file = fc.get_font_file(font_spec);

            // font = pdfWriter->GetFontForFile(font_file, 6);
            // if (!font) font = pdfWriter->GetFontForFile(font_file, 5);
            // if (!font) font = pdfWriter->GetFontForFile(font_file, 4);
            // if (!font) font = pdfWriter->GetFontForFile(font_file, 3);
            // if (!font) font = pdfWriter->GetFontForFile(font_file, 2);
            font = pdfWriter->GetFontForFile(font_file, 0);
            // if (!font || !font->GetFreeTypeFont()->IsValid()) {
            //     std::cout << "Invalid font, trying to load default" << std::endl;
            //     font = pdfWriter->GetFontForFile(font_file, 0);
            // }
            // std::cout << font->GetFreeTypeFont()->GetFontWeight() << std::endl;
            font->GetFreeTypeFont()->GetFontFlags();
            fc.add_font(font_spec, font);

            return font;
        };

        hummus_impl(const page_size size = A4) : pdfWriter(new PDFWriter()), pdfPage(nullptr), pageContentContext(nullptr) {
            create_page(size);
        }

        ~hummus_impl() { delete pdfWriter; }

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
        if (eSuccess != impl->pdfWriter->EndPageContentContext(impl->pageContentContext)) cerr << "Error ending Page Page Context";
        if (eSuccess != impl->pdfWriter->WritePageAndRelease(impl->pdfPage)) cerr << "Error ending PDF Page";
        if (eSuccess != impl->pdfWriter->EndPDF()) cerr << "Error ending PDF";
        delete impl;
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

    bool pdf::DrawText(const string &text, const rect &pos, const double &fontsize, const font_spec &font) {
        const pdf_rect position{pos, impl->height};
        impl->pageContentContext->BT();
        impl->pageContentContext->k(0, 0, 0, 1);
        impl->pageContentContext->Tf(impl->get_font(font), fontsize);
        impl->pageContentContext->Td(position.left(), position.bottom());


        // More advanced with Tm sizing:
        // A - Char width (Value of 2 makes text double wide)
        // B - Rise (value of 1 makes text go higher at 45º)
        // C - Lean (value of 1 makes text very italic at 45º)
        // D - Char height (Value of 2 makes text double high)
        // impl->pageContentContext->Tm(1, 0, 0, 1, position.left(), position.bottom());


        // If center align:
        // auto adv = arialTTF->CalculateTextAdvance(text, fontsize);
        // impl->pageContentContext->Tm(1, 0, 0, 1, rect.center() - adv/2, rect.bottom());


        impl->pageContentContext->Tj(text);

        // impl->pageContentContext->Tr(1);
        // impl->pageContentContext->Tj("Text with 1 rendering mode");

        // impl->pageContentContext->TL(fontsize*1.2);
        // impl->pageContentContext->Tj("Some");
        // impl->pageContentContext->Quote("Multiline");
        // impl->pageContentContext->Quote("Text");


        // GlyphUnicodeMappingList glyphs;
        // glyphs.emplace_back(61, 'Z'); //
        // impl->pageContentContext->Tj(glyphs);

        impl->pageContentContext->ET();
        return true;
    }

    void pdf::NewPage() {
        impl->create_page();
    }

    void pdf::DrawLine(const rect &pos) {
        const pdf_rect rect{pos, impl->height};

        impl->pageContentContext->q();
        impl->pageContentContext->w(2);
        impl->pageContentContext->K(0, 0, 1, 0);

        impl->pageContentContext->m(rect.left(), rect.bottom());
        impl->pageContentContext->l(rect.right(), rect.top());
        impl->pageContentContext->s();

        impl->pageContentContext->Q();
    }

    void pdf::DrawRect(const rect &pos) {
        const pdf_rect rect{pos, impl->height};
        impl->pageContentContext->q();
        impl->pageContentContext->k(0, 1, 0, 0);
        impl->pageContentContext->re(rect.left(), rect.bottom(), rect.width(), rect.height());
        impl->pageContentContext->f();
        impl->pageContentContext->Q();
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
}
