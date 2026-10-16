# simply-cpp-pdf

C++20 PDF writer: pages of text, shapes, paths, images, SVG drawings and
links, on [PDFHummus](https://github.com/galkahana/PDF-Writer).

## Licence, and the other engine

**sc-pdf is Apache-2.0**, a permissive licence, as PDFHummus is: use it in
programs under any licence, closed-source and commercial included.

[sc-mupdf](https://github.com/roelofrossouw/simply-cpp-mupdf) writes PDFs
with the same calls on [MuPDF](https://mupdf.com/), and can do more: read
PDFs (and XPS, EPUB, CBZ), extract their text with positions and fonts, and
render pages to images. But it's **AGPL-3.0**, a stricter licence that brings
the AGPL to any program linking it. Both are an `sc::canvas`, so an app can
switch engines by changing one class name and pick the licence it lives
under: see the wiki's [PDF engines](https://github.com/roelofrossouw/simply-cpp/wiki/PDF-Engines)
page.

**Documentation:** the [simply-cpp wiki](https://github.com/roelofrossouw/simply-cpp/wiki) has a [PDF guide](https://github.com/roelofrossouw/simply-cpp/wiki/PDF) and the [sc-pdf reference](https://github.com/roelofrossouw/simply-cpp/wiki/Reference-sc-pdf), plus [getting started](https://github.com/roelofrossouw/simply-cpp/wiki/Getting-Started) for the whole suite.

An `sc::pdf` is an `sc::canvas` from
[sc-graphics](https://github.com/roelofrossouw/simply-cpp-graphics): shapes,
text, paths and SVG are drawn with the same `draw()` calls as on an
`sc::image`. Positions are in points (1/72 inch) from the page's top-left
corner, x to the right and y down, as on a screen; colours are `sc::color`s,
and a colour's alpha is honoured (fills, strokes and text). Fonts are found
through fontconfig by family, weight and slant, and embedded (subset) in the
file. Circles, rounded corners, curves and SVG stay vector graphics, and SVG
clip paths clip. PDFHummus and fontconfig stay out of the headers.

Pages are `sc::page_size::a3`, `a4` (the default), `a5`, `letter` or `legal`,
portrait or `sc::orientation::landscape` (both from sc-graphics' `page.h`,
shared with sc-mupdf); `sc::page_size_from("a3")` reads a name, as from a
command line.

```cpp
#include <pdf.h>

sc::pdf pdf{"letter.pdf"};                                   // A4 portrait
// sc::pdf wide{"wide.pdf", sc::page_size::a3, sc::orientation::landscape};   // 1191 x 842 points
pdf.draw(sc::text{"Hello", {50, 80}, sc::font::bold("sans-serif", 24)});
const sc::text body{"Body text", {50, 100}, {"serif", 11}};  // baseline at y = 100
pdf.draw(body);
const double width = pdf.text_width(body);                   // in its own font
pdf.draw(sc::line{{50, 110}, {50 + width, 110}}, sc::stroke{sc::color::Black, 0.5});
pdf.draw(sc::rect{50, 130, 200, 80}, sc::color::Black);       // outline: stroke colour
pdf.draw(sc::rounded_rect{sc::rect{300, 130, 200, 80}, 8}, {sc::color::Black, sc::color{0.9, 0.95, 1}}); // + fill
pdf.draw(sc::circle{{400, 300}, 30}, sc::appearance::filled(sc::color{0, 0, 0, 0.5})); // half opaque
pdf.draw_image("logo.png", {300, 130, 200, 80});             // JPEG, PNG or TIFF, fitted
pdf.draw(sc::svg::load("logo.svg"), sc::rect{50, 230, 100, 100}); // vector, fitted
pdf.add_link("https://example.com", {50, 30, 120, 25});
pdf.new_page();
pdf.close();                                                 // or let the destructor finish it
```

`close()` throws `std::runtime_error` when the file can't be written; the
destructor finishes an unclosed file but can only print a failure.
`set_page(n)` goes back to an earlier page (or adds pages up to `n`), so
things like "page 1 of 3" can be filled in at the end.

`sc::svg` (sc-graphics) reads paths, the basic shapes (including rounded
`rect`s), groups, transforms, clip paths, `<defs>`/`<use>` and flat fill and
stroke colours: icons and logos, not gradients, text or filters.

## Demo

`sc-pdf-demo` writes a PDF (a heading, text measured to centre it, lines,
shapes, SVG drawings with transforms, an even-odd hole and a clip path, a
link, and a face path placed in the bottom-right corner by its `bounds()`,
with half- and three-quarter-opaque eyes), showing each call. It is installed
with the runtime package (`simply-cpp-pdf`):

```text
sc-pdf-demo [--size a4|a3|letter|legal] [--landscape] [--svg FILE] [OUTPUT]
```

```bash
sc-pdf-demo                              # A4, sc-pdf-demo.pdf in the temporary directory
sc-pdf-demo --size letter --landscape wide.pdf
sc-pdf-demo --svg logo.svg               # adds a page with logo.svg fitted to it
sc-pdf-demo --help
```

A page too short for the SVG frames (Letter, or landscape) puts them on a
second page.

Its source is `examples/sc-pdf-demo.cpp`; the code below is copied from it at
configure time, so it always matches code that compiles:

<!-- sc-example: examples/sc-pdf-demo.cpp -->
```cpp
sc::console::heading("Starting the file");
sc::pdf pdf{output, paper, turned};
sc::console::step("sc::pdf pdf{output, paper, turned};   // " + output.string());
SC_SHOW(pdf.page_width());  // points, 1/72 inch: --size and --landscape
SC_SHOW(pdf.page_height());

sc::console::heading("Text: sc::text{content, where its baseline starts, sc::font}");
SC_STEP(pdf.draw(sc::text{"simply-cpp pdf", {50, 90}, sc::font::bold("sans-serif", 32)}));
SC_STEP(pdf.draw(sc::text{"Text, shapes and SVG on PDFHummus", {50, 120}, {"serif", 14, 400, true}}));
sc::text centred{"This line is centred on the page", {0, 150}}; // the default font, sans-serif 12
SC_SHOW(pdf.text_width(centred));
centred.position.x((pdf.page_width() - pdf.text_width(centred)) / 2);
SC_STEP(pdf.draw(centred, sc::color{0.2, 0.2, 0.6}));

sc::console::heading("Lines and shapes: draw(geometry, {stroke, fill})");
SC_STEP(pdf.draw(sc::line{{50, 165}, {pdf.page_width() - 50, 165}}, sc::stroke{sc::color::Black, 0.5}));
const sc::rect box{50, 190, 230, 120};
SC_STEP(pdf.draw(sc::rounded_rect{box, 12}, {sc::stroke{sc::color{0.16, 0.44, 0.59}, 2}, sc::color{0.85, 0.93, 0.98}})); // stroke, fill
SC_STEP(pdf.draw(sc::text{"A rounded box", {70, 255}, {"sans-serif", 16}}));
SC_STEP(pdf.draw(sc::circle{{110, 400}, 40}, {sc::stroke{sc::color::Black, 2}, sc::color{0.96, 0.83, 0.37}}));
SC_STEP(pdf.draw(sc::polygon{{180, 440}, {230, 360}, {280, 440}}, sc::stroke{sc::color{0.93, 0.42, 0.3}, 3}));
SC_STEP(pdf.draw(sc::rotated_rect{{380, 400}, {120, 50}, 20}, {sc::color::Black, sc::color{0.85, 0.93, 0.98}}));

sc::console::heading("An SVG drawing, scaled to fit and centred");
const std::string svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="320" height="200">)"
                        R"(<rect width="320" height="200" rx="16" fill="#2a6f97"/>)"
                        R"(<circle cx="160" cy="100" r="70" fill="#f4d35e"/>)"
                        R"(<path d="M120 130 Q160 170 200 130" stroke="#2a6f97" stroke-width="8" fill="none"/></svg>)";
SC_STEP(pdf.draw(sc::svg{svg}, sc::rect{315, 190, 230, 120})); // sc-graphics reads it into paths

sc::console::heading("A link");
SC_STEP(pdf.draw(sc::text{"github.com/roelofrossouw/simply-cpp", {50, 490}}, sc::color::Blue));
SC_STEP(pdf.add_link("https://github.com/roelofrossouw/simply-cpp", {50, 476, 220, 18}));

sc::console::heading("More SVG: transforms, arcs with an even-odd hole, a clip path");
const sc::svg star{R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">)svg"
                   R"svg(<g transform="rotate(15 50 50)"><polygon fill="#e76f51" stroke="#264653" stroke-width="3")svg"
                   R"svg( points="50,5 61,38 95,38 67,58 78,92 50,72 22,92 33,58 5,38 39,38"/></g></svg>)svg"};
const sc::svg ring{R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">)svg"
                   R"svg(<path fill="#2a9d8f" fill-rule="evenodd" d="M5 50 A45 45 0 1 1 95 50 A45 45 0 1 1 5 50 Z)svg"
                   R"svg( M25 50 A25 25 0 1 0 75 50 A25 25 0 1 0 25 50 Z"/></svg>)svg"};
const sc::svg disc{R"svg(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">)svg"
                   R"svg(<defs><clipPath id="round"><circle cx="50" cy="50" r="45"/></clipPath></defs>)svg"
                   R"svg(<rect clip-path="url(#round)" width="100" height="50" fill="#e9c46a"/>)svg"
                   R"svg(<rect clip-path="url(#round)" y="50" width="100" height="50" fill="#264653"/></svg>)svg"};
double top = 530, left_edge = 50;
if (top + 300 > pdf.page_height()) { // no room on a shorter page (Letter, landscape): the frames go on the next
    SC_STEP(pdf.new_page());
    top = 50;
}
for (const auto &[drawing, caption]: {std::pair{&star, "rotate(15)"}, std::pair{&ring, "evenodd"}, std::pair{&disc, "clip-path"}}) {
    const sc::rect frame{left_edge, top, 150, 150};
    pdf.draw(frame, sc::stroke{sc::color{0.8, 0.8, 0.8}, 0.5});
    pdf.draw(*drawing, frame - 10); // 10 points inside the frame
    pdf.draw(sc::text{caption, {left_edge, top + 168}, {"monospace", 10}});
    left_edge += 172.5;
}
sc::console::step("pdf.draw(star / ring / disc, frame - 10);   // three 150-point frames");

sc::console::heading("A path in the bottom-right corner, placed by its bounds()");
// A face drawn round (0, 0), with a curl of hair whose control points reach higher than the
// curve does.
auto face = to_path(sc::circle{{0, 0}, 40});
face.move_to({-22, 12}).cubic_to({-10, 28}, {10, 28}, {22, 12});  // the smile
face.move_to({-8, -40}).cubic_to({-8, -60}, {8, -60}, {8, -40});  // the curl
SC_SHOW(face.bounds()); // exact: the curl's top is at -55, not its control points' -60
const sc::point corner{pdf.page_width() - 30, pdf.page_height() - 30};  // 30 points in from the corner
const sc::point offset = corner - face.bounds().right_bottom();         // moves the bounds' corner there
SC_STEP(pdf.draw(face.transformed(1, 1, offset), sc::stroke{sc::color::Black, 2}));
// Transparency is the colour's alpha: one eye half opaque, the other three-quarters.
SC_STEP(pdf.draw(sc::circle{offset + sc::point{-14, -10}, 6}, sc::appearance::filled(sc::color{0.15, 0.27, 0.33, 0.5})));
SC_STEP(pdf.draw(sc::circle{offset + sc::point{14, -10}, 6}, sc::appearance::filled(sc::color{0.15, 0.27, 0.33, 0.75})));

sc::console::heading("Finishing the file");
if (!svg_file.empty()) {
    SC_STEP(pdf.new_page());
    const auto drawing = sc::svg::load(svg_file);
    sc::console::step("const auto drawing = sc::svg::load(svg_file);   // " + svg_file.string());
    SC_SHOW(drawing.elements().size());
    SC_STEP(pdf.draw(drawing, sc::rect{50, 50, pdf.page_width() - 100, pdf.page_height() - 100}));
}
SC_SHOW(pdf.page_count());
SC_STEP(pdf.close());
SC_SHOW(std::filesystem::file_size(output)); // bytes
```
<!-- /sc-example -->

## Dependencies

PDFHummus isn't packaged by Ubuntu or Homebrew, so
[simply-cpp-pdfhummus](https://github.com/roelofrossouw/simply-cpp-pdfhummus)
builds it from source for both. sc-graphics brings the canvas, page sizes and
SVG reading (on tinyxml2); fontconfig (finding fonts) comes from the system. A
machine running a program needs some fonts installed; Ubuntu's fontconfig
brings DejaVu.

## Building locally

Run `bash scripts/install.sh` to configure, build, test, package, and install.

## Usage

```cmake
find_package(sc-pdf CONFIG REQUIRED)

target_link_libraries(myapp PRIVATE sc::sc-pdf-shared)
```

```bash
sudo curl -fsSL https://apt.roelof.co.za/setup.sh | bash
sudo apt -y install simply-cpp-pdf-dev
```
