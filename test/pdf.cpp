#include "pdf.h"

void draw_content_on_pdf_page(sc::pdf &pdf, sc::rect &pos) {
    pdf.StrokeRect(pos + 5, 10, {1, 0, 0, 0.3});
    pdf.FillRect(pos - 10, {0, 1, 0, 0.3});
    pdf.DrawLine(pos - 5, 2, {0, 0, 1, 0.3});
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
    sc::pdf pdf{"../output.pdf"};
    sc::rect pos{100, 100, 100, 100};
    draw_content_on_pdf_page(pdf, pos);
    // pdf.NewPage();
    // pos = sc::rect{25, 25, 50, 50};
    // draw_content_on_pdf_page(pdf, pos);

    return 0;
}
