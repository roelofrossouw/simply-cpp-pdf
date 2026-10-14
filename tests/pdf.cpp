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

            CHECK_NOTHROW(pdf.draw(sc::text{"Hello, PDF", {50, 80}, {"sans-serif", 24}}));
            CHECK_NOTHROW(pdf.draw(sc::text{"Bold and blue", {50, 110}, sc::font::bold("sans-serif", 14)}, sc::color{0, 0, 1}));
            CHECK_NOTHROW(pdf.draw(sc::line{{50, 120}, {545, 120}}, sc::color::Black, 0.5));
            CHECK_NOTHROW(pdf.draw(sc::rect{50, 140, 200, 100}, sc::color::Red, sc::color::Transparent, 2, 12));
            CHECK_NOTHROW(pdf.draw(sc::rect{300, 140, 200, 100}, sc::color::Transparent, sc::color{0.6, 0.95, 0.4}));
            CHECK_NOTHROW(pdf.draw(sc::rect{300, 140, 200, 100}, sc::color::Black, sc::color::White, 1, 6));
            CHECK_NOTHROW(pdf.draw(sc::circle{{400, 600}, 40}, sc::color::Black, sc::color{1, 0.8, 0.2}, 2));
            CHECK_NOTHROW(pdf.draw(sc::polygon{{300, 700}, {360, 640}, {420, 700}}, sc::color::Blue));
            CHECK_NOTHROW(pdf.draw(sc::polygon{{300.5, 760.25}, {360.75, 700.5}, {420.25, 760.75}}, sc::color::Blue));
            CHECK_NOTHROW(pdf.draw(sc::polygon_i{{300, 800}, {360, 740}, {420, 800}}, sc::color::Black));
            CHECK_NOTHROW(pdf.draw(sc::rotated_rect{{480, 600}, {60, 30}, 30}, sc::color::Transparent, sc::color::Green));
            CHECK_NOTHROW(pdf.draw(sc::polygon{}, sc::color::Black)); // nothing to draw
            CHECK_NOTHROW(pdf.draw(sc::circle{{0, 0}, 0}, sc::color::Black));
            CHECK_NOTHROW(pdf.draw_image("resource/image.png", {50, 260, 240, 160}));
            CHECK_NOTHROW(pdf.draw_image("resource/image.jpg", {300, 260, 240, 160}));
            CHECK_NOTHROW(pdf.draw_svg(badge, {50, 440, 200, 100}));
            CHECK_NOTHROW(pdf.add_link("https://github.com/roelofrossouw/simply-cpp", {50, 60, 150, 30}));
            pdf.close();
            CHECK_NOTHROW(pdf.close()); // twice does nothing
            CHECK_THROWS_AS(pdf.draw(sc::text{"too late", {50, 600}}), std::logic_error);
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
            pdf.draw(sc::text{"Page four", {72, 72}});
            pdf.set_page(1);
            pdf.draw(sc::text{"Back on page one", {72, 72}});
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
        const double narrow = pdf.text_width(sc::text{"iii"});
        const double wide = pdf.text_width(sc::text{"WWW", {}, {"sans-serif", 12}});
        CHECK_LT(0.0, narrow);
        CHECK_LT(narrow, wide);
        CHECK_NEAR(pdf.text_width(sc::text{"WWW", {300, 400}, {"sans-serif", 24}}), 2 * wide, 0.01); // position doesn't matter
        CHECK_EQ(pdf.text_width(sc::text{}), 0.0);
        const double x_height = pdf.text_x_height({"sans-serif", 100});
        const double cap_height = pdf.text_cap_height({"sans-serif", 100});
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
            second.draw(sc::text{"Moved", {72, 72}});
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
