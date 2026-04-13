#include "pdf.h"

#include <vector>

#include "svg_element.h"
#include "../third_party/sc/sc.h"

void draw_content_on_pdf_page(sc::pdf &pdf, sc::rect &pos) {
    sc::rect sp{pos.left(), pos.top(), 130, 90};
    // sc::rect sp{pos.left(), pos.top(), 400, 300};
    double padding = 5;
    std::vector<std::string> glyphs{
        "file-earmark-pdf-bs", "flag_hk", "square-empty", "calendar-day-bs", "flag_za", "camping-benches", "1web",
        "sun-bs", "syringe-empty", "flag_mu", "hamburger", "truck-bs", "sun", "sunrise-fill-bs", "bootstrap-reboot-bs",
        "natis", "cup-hot-bs", "flag_mq", "flag_ch", "flag_it", "flag_tz", "flag_az", "flag_sa", "flag_bw", "flag_is",
        "flag_gh", "flag_in", "flag_ag", "quora-bs", "flag_br"
    };
    // glyphs = {"camping-benches"};
    // glyphs = {"flag_mq"};
    for (auto &glyph: glyphs) {
        std::string svg_data;
        svg_data = sc::rest::fetch("https://roelof.1web.co.za/images/glyph.svg?g=" + glyph + "&v=2");
        // svg_data = sc::file_get_contents("/Users/roelof/simply-cpp-pdf/test/resource/test.svg");
        // std::cout << svg_data << std::endl;

        pdf.FillRect(sp + 1, {0.6, 0.95, 0.4});
        pdf.DrawObject(svg_data, sp);
        pdf.StrokeRect(sp + 1, 1);
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
    sc::rect pos{30, 30, 150, 150};
    draw_content_on_pdf_page(*pdf, pos);
    // pdf->NewPage();
    // pos = sc::rect{25, 25, 50, 50};
    // draw_content_on_pdf_page(*pdf, pos);
    delete pdf;
    std::cout << "Done in " << t << std::endl;

    return 0;
}
