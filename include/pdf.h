#ifndef SC_PDF_H
#define SC_PDF_H
#include "base_types.h"
#include "font_spec.h"


namespace sc {
    class hummus_impl;

    enum page_size {
        Default = 0,
        A4 = 1,
        A5 = 2,
        Letter = 3,
        Legal = 4,
        Custom = 5
    };

    class pdf {
    public:
        explicit pdf(const std::string &filename, page_size size = A4);

        explicit pdf(const char *filename);

        ~pdf();

        bool DrawImage(const std::string &filename);

        bool MoreText(const std::string &text, const rect &pos);

        bool DrawText(const std::string &text, const rect &pos, const double &fontsize = 10, const font_spec &font = font_spec("Roboto"),
                      const color &col = color(0, 0, 0));

        void NewPage();

        auto DrawLine(const rect &pos, double width = 2, const color &col = {0, 0, 0, 1}) -> void;

        void StrokeRect(const rect &pos, double width = 2, const color &col = {0, 0, 0, 1});

        void FillRect(const rect &pos, const color &col = {0, 0, 0, 1});

        void DrawObject();

        void AddLink(const std::string &url, const rect &pos);

        void Merge();

        double PageHeight() const;

        double PageWidth() const;

        void TestEmoji() const;

        double TextWidth(const std::string &text, const font_spec &font, float size) const;

        double TextXHeight(const font_spec &font) const;

        double TextCapHeight(const font_spec &font) const;

        void SetPage(unsigned int page_number);

        unsigned int PageCount() const;

    private:
        hummus_impl *impl{};
    };
}


#endif //SC_PDF_H
