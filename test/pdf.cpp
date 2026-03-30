#include "pdf.h"

#include <vector>

#include "../third_party/sc/sc.h"

void draw_content_on_pdf_page(sc::pdf &pdf, sc::rect &pos) {
    // pdf.FillRect({10, 10, 100, 100}, {0, 1, 0, 0.3});

    sc::rect sp{pos};
    double padding = 10;
    std::vector<std::string> glyphs{"file-earmark-pdf-bs", "flag_hk", "square-empty", "calendar-day-bs", "flag_za", "camping-benches", "1web"};
    // std::vector<std::string> glyphs{"square-empty"};
    for (auto &glyph: glyphs) {
        const auto svg_data = sc::rest::fetch("https://roelof.1web.co.za/images/glyph.svg?g=" + glyph);
        // std::cout << svg_data << std::endl;
        pdf.StrokeRect(sp + 1, 1);

        pdf.DrawObject(svg_data, sp);
        sp += {0, sp.height() + padding};
        if (sp.bottom() > pdf.PageHeight() - pos.top()) {
            sp = {sp.left() + sp.width() + padding, pos.top(), sp.width(), sp.height()};
        }
    }
    return;

    pdf.StrokeRect(pos, 3, {1, 0, 0, 0.3}, 25);
    pdf.FillRect(pos - 3, {0, 1, 0, 0.3}, 22);
    pdf.DrawLine(pos - 12.5, 2, {0, 0, 1, 0.3});
    pdf.DrawLine({pos.left(), pos.middle(), pos.width()}, 5, {0.5, 0.5, 0, 0});
    pdf.AddLink("http://www.pdfhummus.com", pos);

    std::string font_family = "Roboto";
    sc::rect pos2{pos.left(), pos.top(), 0, 0};
    for (int weight = 100; weight <= 900; weight += 100) {
        pos2 += sc::rect{0, 20};
        pdf.DrawText(font_family + " " + std::to_string(weight) + " Italics", pos2, 16, {font_family, weight, true, ""});
    }
    pos2 = {pos.left(), pos.top(), 0, 0};
    pos2 += {200, 0};
    for (int weight = 100; weight <= 900; weight += 100) {
        pos2 += sc::rect{0, 20};
        pdf.DrawText(font_family + " " + std::to_string(weight) + "", pos2, 16, {font_family, weight, false, ""});
    }

    font_family = "Arial";
    pos2 = {pos.left(), pos.top(), 0, 0};
    pos2 += {0, 200};
    for (int weight = 100; weight <= 900; weight += 100) {
        pos2 += sc::rect{0, 20};
        pdf.DrawText(font_family + " " + std::to_string(weight) + " Italics", pos2, 16, {font_family, weight, true, ""});
    }
    pos2 = {pos.left(), pos.top(), 0, 0};
    pos2 += {200, 200};
    for (int weight = 100; weight <= 900; weight += 100) {
        pos2 += sc::rect{0, 20};
        pdf.DrawText(font_family + " " + std::to_string(weight) + "", pos2, 16, {font_family, weight, false, ""});
    }

    pdf.TestEmoji();
}

int main(int argc, char *argv[]) {
    sc::timer t;
    auto *pdf = new sc::pdf{"../output.pdf"};
    sc::rect pos{50, 50, 150, 150};
    draw_content_on_pdf_page(*pdf, pos);
    // pdf->NewPage();
    // pos = sc::rect{25, 25, 50, 50};
    // draw_content_on_pdf_page(*pdf, pos);
    delete pdf;
    std::cout << "Done in " << t << std::endl;

    return 0;
}
