// Writes a one-page PDF: a heading, text measured to centre it, lines, rounded rectangles, an SVG
// drawing and a link. Each line shows a call, as written, and what it returned. Pass a file name to
// write there instead of the temporary directory.

#include <filesystem>
#include <iostream>
#include <string>

#include <sc.h>

#include <pdf.h>

int main(int argc, char **argv) {
    try {
        const std::filesystem::path output = argc > 1 ? argv[1] : std::filesystem::temp_directory_path() / "sc-pdf-demo.pdf";
        sc::console::title("simply-cpp pdf: text, shapes, an SVG drawing and a link on an A4 page");
        sc::timer sw;

        // [readme]
        sc::console::heading("Starting the file");
        sc::pdf pdf{output};
        sc::console::step("sc::pdf pdf{output};   // " + output.string());
        SC_SHOW(pdf.page_width());  // points, 1/72 inch: A4 by default
        SC_SHOW(pdf.page_height());

        sc::console::heading("Text: positions are from the top left, the baseline along the bottom");
        SC_STEP(pdf.draw_text("simply-cpp pdf", {50, 50, 0, 40}, 32, sc::font_spec::bold()));
        SC_STEP(pdf.draw_text("Text, shapes and SVG on PDFHummus", {50, 120}, 14, {"serif", 400, true}));
        const std::string centred = "This line is centred on the page";
        SC_SHOW(pdf.text_width(centred, {}, 12));
        const double left = (pdf.page_width() - pdf.text_width(centred, {}, 12)) / 2;
        SC_STEP(pdf.draw_text(centred, {left, 150}, 12, {}, sc::color{0.2, 0.2, 0.6}));

        sc::console::heading("Lines and shapes: draw(geometry, stroke, fill, width)");
        SC_STEP(pdf.draw_line({50, 165}, {545, 165}, 0.5));
        const sc::rect box{50, 190, 230, 120};
        SC_STEP(pdf.draw(box, sc::color{0.16, 0.44, 0.59}, sc::color{0.85, 0.93, 0.98}, 2, 12)); // stroke, fill, width, radius
        SC_STEP(pdf.draw_text("A rounded box", {70, 255}, 16));
        SC_STEP(pdf.draw(sc::circle{{110, 400}, 40}, sc::color::Black, sc::color{0.96, 0.83, 0.37}, 2));
        SC_STEP(pdf.draw(sc::polygon{{180, 440}, {230, 360}, {280, 440}}, sc::color{0.93, 0.42, 0.3}, sc::color::Transparent, 3));
        SC_STEP(pdf.draw(sc::rotated_rect{{380, 400}, {120, 50}, 20}, sc::color::Black, sc::color{0.85, 0.93, 0.98}));

        sc::console::heading("An SVG drawing, scaled to fit and centred");
        const std::string svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="320" height="200">)"
                                R"(<rect width="320" height="200" rx="16" fill="#2a6f97"/>)"
                                R"(<circle cx="160" cy="100" r="70" fill="#f4d35e"/>)"
                                R"(<path d="M120 130 Q160 170 200 130" stroke="#2a6f97" stroke-width="8" fill="none"/></svg>)";
        SC_STEP(pdf.draw_svg(svg, {315, 190, 230, 120}));

        sc::console::heading("A link and finishing the file");
        SC_STEP(pdf.draw_text("github.com/roelofrossouw/simply-cpp", {50, 490}, 12, {}, sc::color::Blue));
        SC_STEP(pdf.add_link("https://github.com/roelofrossouw/simply-cpp", {50, 476, 220, 18}));
        SC_SHOW(pdf.page_count());
        SC_STEP(pdf.close());
        SC_SHOW(std::filesystem::file_size(output)); // bytes
        // [/readme]

        sc::console::output() << "\nThe result is " << output.string() << "\nAll of that took " << sw << ".\n";
    } catch (const std::exception &error) {
        std::cerr << "sc-pdf-demo: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
