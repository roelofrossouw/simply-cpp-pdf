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
        // std::string svg_data = std::string("<svg viewBox='0 0 900 600' >") +
        //                        std::string("<rect width=\"900\" height=\"600\" fill=\"#de2910\"/>") +
        //                        std::string("<g id=\"petal\">") +
        //                        std::string("<path d=\"m 449.96406,299.9134 c -105.26305,-44.48626 -58.60174,-181.58185 42.06956,-174.6907") +
        //                        std::string(" -20.36609,10.46694 -23.31775,29.99772 -11.68704,48.09021 13.02444,20.2558 -1.19897,52.84856") +
        //                        std::string(" -18.80577,60.7674 -28.93485,13.02443 -34.72791,47.74999 -11.57675,65.83309 z\" fill=\"#fff\"/>") +
        //                        std::string("<path d=\"m 444.27188,200.91974 -5.91976,9.29378 -2.14454,-10.8142 -10.67812,-2.75928") +
        //                        std::string(" 9.62461,-5.3895 -0.67104,-10.99955 8.08542,7.48945 10.25578,-4.04271 -4.61053,10.00942") +
        //                        std::string(" 7.00143,8.50541 z\" fill=\"#de2910\"/>") +
        //                        std::string("<path d=\"m 450.56002,298.75902 c -12.73114,-6.53451 -22.9963,-20.15491 -27.46839,-36.43134") +
        //                        std::string(" -5.11498,-18.66969 -2.17269,-38.74247 8.08308,-55.03768 l -2.20789,-1.39371 c -10.64057,16.92871 -13.69313,37.74293") +
        //                        std::string(" -8.38575,57.11886 4.72784,17.22201 15.21355,31.09815 28.78703,38.06438 z\" fill=\"#de2910\"/>") +
        //                        std::string("</g>") +
        //                        std::string("<use xlink:href=\"#petal\" transform=\"rotate(72 450,300)\"/>") +
        //                        std::string("<use xlink:href=\"#petal\" transform=\"rotate(144 450,300)\"/>") +
        //                        std::string("<use xlink:href=\"#petal\" transform=\"rotate(216 450,300)\"/>") +
        //                        std::string("<use xlink:href=\"#petal\" transform=\"rotate(288 450,300)\"/>") +
        //                        std::string("<path  d=\"\" />") +
        //                        std::string("</svg>");


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
