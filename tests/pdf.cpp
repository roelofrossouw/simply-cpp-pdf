// Writes PDFs into the temporary directory and checks what the files hold: pages, sizes, links,
// fonts, images and SVG drawings, plus the mistakes that throw.
#include "pdf.h"

#include "sc_test.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
    const auto directory = std::filesystem::temp_directory_path() / "sc-pdf-test";

    std::string read(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::binary};
        return {std::istreambuf_iterator<char>{file}, {}};
    }

    bool contains(const std::string &text, const std::string &part) { return text.find(part) != std::string::npos; }

    const std::string badge = R"(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="100" viewBox="0 0 200 100">)"
                              R"(<rect x="5" y="5" width="190" height="90" rx="12" fill="#2a6f97" stroke="#000000"/>)"
                              R"(<circle cx="50" cy="50" r="30" fill="#f4d35e"/>)"
                              R"(<path d="M100 20 L180 20 L140 80 Z" fill="#ee6c4d"/></svg>)";
}

int main() {
    std::filesystem::create_directories(directory);

    SECTION("A page of everything");
    {
        const auto path = directory / "everything.pdf";
        {
            sc::pdf pdf{path};
            CHECK_EQ(pdf.page_count(), 1u);
            CHECK_EQ(pdf.page_width(), 595.0);
            CHECK_EQ(pdf.page_height(), 842.0);

            CHECK_NOTHROW(pdf.draw_text("Hello, PDF", {50, 80}, 24));
            CHECK_NOTHROW(pdf.draw_text("Bold and blue", {50, 110}, 14, sc::font_spec::bold(), sc::color{0, 0, 1}));
            CHECK_NOTHROW(pdf.draw_line({50, 120}, {545, 120}, 0.5));
            CHECK_NOTHROW(pdf.stroke_rect({50, 140, 200, 100}, 2, sc::color::Red, 12));
            CHECK_NOTHROW(pdf.fill_rect({300, 140, 200, 100}, sc::color{0.6, 0.95, 0.4}));
            CHECK_NOTHROW(pdf.draw_image("resource/image.png", {50, 260, 240, 160}));
            CHECK_NOTHROW(pdf.draw_image("resource/image.jpg", {300, 260, 240, 160}));
            CHECK_NOTHROW(pdf.draw_svg(badge, {50, 440, 200, 100}));
            CHECK_NOTHROW(pdf.add_link("https://github.com/roelofrossouw/simply-cpp", {50, 60, 150, 30}));
            pdf.close();
            CHECK_NOTHROW(pdf.close()); // twice does nothing
            CHECK_THROWS_AS(pdf.draw_text("too late", {50, 600}), std::logic_error);
        }
        const auto file = read(path);
        CHECK(file.starts_with("%PDF-1.4"));
        CHECK(contains(file, "%%EOF"));
        CHECK(contains(file, "/Count 1"));
        CHECK(contains(file, "/MediaBox [ 0 0 595 842 ]"));
        CHECK(contains(file, "/URI (https://github.com/roelofrossouw/simply-cpp)"));
        CHECK(contains(file, "/FontFile")); // the font is embedded
        CHECK(contains(file, "/Subtype /Image"));
        CHECK(contains(file, "/Subtype /Form")); // the SVG
    }

    SECTION("Pages");
    {
        const auto path = directory / "pages.pdf";
        {
            sc::pdf pdf{path, sc::page_size::letter};
            CHECK_EQ(pdf.page_width(), 612.0);
            CHECK_EQ(pdf.page_height(), 792.0);
            pdf.new_page();
            CHECK_EQ(pdf.page(), 2u);
            pdf.set_page(4); // adds pages 3 and 4
            CHECK_EQ(pdf.page_count(), 4u);
            CHECK_EQ(pdf.page(), 4u);
            pdf.draw_text("Page four", {72, 72});
            pdf.set_page(1);
            pdf.draw_text("Back on page one", {72, 72});
            pdf.add_link("https://example.com/one", {72, 60, 100, 20});
            CHECK_THROWS_AS(pdf.set_page(0), std::out_of_range);
        }
        const auto file = read(path);
        CHECK(contains(file, "/Count 4"));
        CHECK(contains(file, "/MediaBox [ 0 0 612 792 ]"));
        CHECK(contains(file, "/URI (https://example.com/one)"));

        sc::pdf legal{directory / "legal.pdf", sc::page_size::legal};
        CHECK_EQ(legal.page_height(), 1008.0);
        sc::pdf a5{directory / "a5.pdf", sc::page_size::a5};
        CHECK_EQ(a5.page_width(), 420.0);
        CHECK_EQ(a5.page_height(), 595.0);
    }

    SECTION("Text measurements");
    {
        sc::pdf pdf{directory / "measure.pdf"};
        const double narrow = pdf.text_width("iii", {}, 12);
        const double wide = pdf.text_width("WWW", {}, 12);
        CHECK_LT(0.0, narrow);
        CHECK_LT(narrow, wide);
        CHECK_NEAR(pdf.text_width("WWW", {}, 24), 2 * wide, 0.01);
        const double x_height = pdf.text_x_height({}, 100);
        const double cap_height = pdf.text_cap_height({}, 100);
        CHECK_BETWEEN(x_height, 30.0, 70.0);
        CHECK_LT(x_height, cap_height);
        CHECK_BETWEEN(cap_height, 50.0, 90.0);
    }

    SECTION("Moving a pdf");
    {
        const auto path = directory / "moved.pdf";
        {
            sc::pdf first{path};
            sc::pdf second{std::move(first)};
            second.draw_text("Moved", {72, 72});
        }
        CHECK(read(path).starts_with("%PDF-1.4"));
    }

    SECTION("Mistakes");
    {
        CHECK_THROWS_AS(sc::pdf(directory / "no-such-directory" / "x.pdf"), std::runtime_error);
        sc::pdf pdf{directory / "mistakes.pdf"};
        CHECK_THROWS_AS(pdf.draw_image("resource/no-such-image.png", {0, 0, 100, 100}), std::runtime_error);
        CHECK_THROWS_AS(pdf.draw_svg("not svg at all", {0, 0, 100, 100}), std::runtime_error);
    }

    std::filesystem::remove_all(directory);
    TEST_SUMMARY();
}
