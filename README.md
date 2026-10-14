# simply-cpp-pdf

C++20 PDF writer: pages of text, lines, rectangles, images, SVG drawings and
links, on [PDFHummus](https://github.com/galkahana/PDF-Writer).

**Documentation:** the [simply-cpp wiki](https://github.com/roelofrossouw/simply-cpp/wiki) has a [PDF guide](https://github.com/roelofrossouw/simply-cpp/wiki/PDF) and the [sc-pdf reference](https://github.com/roelofrossouw/simply-cpp/wiki/Reference-sc-pdf), plus [getting started](https://github.com/roelofrossouw/simply-cpp/wiki/Getting-Started) for the whole suite.

Positions are `sc::rect`s in points (1/72 inch) from the page's top-left
corner, x to the right and y down, as on a screen; colours are `sc::color`s.
Fonts are found through fontconfig by family, weight and slant, and embedded
(subset) in the file. PDFHummus, tinyxml2 and fontconfig stay out of the
headers.

```cpp
#include <pdf.h>

sc::pdf pdf{"letter.pdf"};                                   // A4; sc::page_size::letter etc.
pdf.draw(sc::text{"Hello", {50, 80}, sc::font::bold("sans-serif", 24)});
const sc::text body{"Body text", {50, 100}, {"serif", 11}};  // baseline at y = 100
pdf.draw(body);
const double width = pdf.text_width(body);                   // in its own font
pdf.draw(sc::line{{50, 110}, {50 + width, 110}}, sc::color::Black, 0.5);
pdf.draw(sc::rect{50, 130, 200, 80}, sc::color::Black);       // outline: stroke colour
pdf.draw(sc::rect{300, 130, 200, 80}, sc::color::Black, sc::color{0.9, 0.95, 1}, 1, 8); // + fill, width, radius
pdf.draw_image("logo.png", {300, 130, 200, 80});             // JPEG, PNG or TIFF, fitted
pdf.draw_svg(svg_text, {50, 230, 100, 100});                 // vector, fitted
pdf.add_link("https://example.com", {50, 30, 120, 25});
pdf.new_page();
pdf.close();                                                 // or let the destructor finish it
```

`close()` throws `std::runtime_error` when the file can't be written; the
destructor finishes an unclosed file but can only print a failure.
`set_page(n)` goes back to an earlier page (or adds pages up to `n`), so
things like "page 1 of 3" can be filled in at the end.

The SVG support covers paths, the basic shapes (including rounded `rect`s),
groups, transforms, clip paths, `<defs>`/`<use>` and flat fill and stroke
colours: icons and logos, not gradients, text or filters.

## Demo

`sc-pdf-demo` writes a one-page PDF (a heading, text measured to centre it,
lines, rounded rectangles, an SVG drawing and a link), showing each call. It
is installed with the runtime package (`simply-cpp-pdf`):

```bash
sc-pdf-demo                 # writes sc-pdf-demo.pdf in the temporary directory
sc-pdf-demo letter.pdf
```

Its source is `examples/sc-pdf-demo.cpp`; the code below is copied from it at
configure time, so it always matches code that compiles:

<!-- sc-example: examples/sc-pdf-demo.cpp -->
```cpp
sc::console::heading("Starting the file");
sc::pdf pdf{output};
sc::console::step("sc::pdf pdf{output};   // " + output.string());
SC_SHOW(pdf.page_width());  // points, 1/72 inch: A4 by default
SC_SHOW(pdf.page_height());

sc::console::heading("Text: sc::text{content, where its baseline starts, sc::font}");
SC_STEP(pdf.draw(sc::text{"simply-cpp pdf", {50, 90}, sc::font::bold("sans-serif", 32)}));
SC_STEP(pdf.draw(sc::text{"Text, shapes and SVG on PDFHummus", {50, 120}, {"serif", 14, 400, true}}));
sc::text centred{"This line is centred on the page", {0, 150}}; // the default font, sans-serif 12
SC_SHOW(pdf.text_width(centred));
centred.position.x((pdf.page_width() - pdf.text_width(centred)) / 2);
SC_STEP(pdf.draw(centred, sc::color{0.2, 0.2, 0.6}));

sc::console::heading("Lines and shapes: draw(geometry, stroke, fill, width)");
SC_STEP(pdf.draw(sc::line{{50, 165}, {545, 165}}, sc::color::Black, 0.5));
const sc::rect box{50, 190, 230, 120};
SC_STEP(pdf.draw(box, sc::color{0.16, 0.44, 0.59}, sc::color{0.85, 0.93, 0.98}, 2, 12)); // stroke, fill, width, radius
SC_STEP(pdf.draw(sc::text{"A rounded box", {70, 255}, {"sans-serif", 16}}));
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
SC_STEP(pdf.draw(sc::text{"github.com/roelofrossouw/simply-cpp", {50, 490}}, sc::color::Blue));
SC_STEP(pdf.add_link("https://github.com/roelofrossouw/simply-cpp", {50, 476, 220, 18}));
SC_SHOW(pdf.page_count());
SC_STEP(pdf.close());
SC_SHOW(std::filesystem::file_size(output)); // bytes
```
<!-- /sc-example -->

## Dependencies

PDFHummus isn't packaged by Ubuntu or Homebrew, so
[simply-cpp-pdfhummus](https://github.com/roelofrossouw/simply-cpp-pdfhummus)
builds it from source for both. tinyxml2 (reading SVG) and fontconfig (finding
fonts) come from the system. A machine running a program needs some fonts
installed; Ubuntu's fontconfig brings DejaVu.

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
