#ifndef SC_PDF_ENGINE_OBJECTS_H
#define SC_PDF_ENGINE_OBJECTS_H

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

// sc-pdf's own PDF writer, written to ISO 32000-1 (PDF 1.7) with no library but zlib. This file:
// how values are spelt, and the file itself (objects, cross-reference table, trailer).
namespace sc::pdf_engine {
    // A number as PDF reads it: an integer when whole, else at most four decimals, never an
    // exponent (PDF has none), "0" rather than "-0". Throws std::invalid_argument when it isn't
    // finite or lies outside what readers must handle (ISO 32000-1 annex C: about ±3.4e38, and
    // integers within 32 bits).
    std::string number(double value);

    // A name: "/Helvetica"; bytes outside ! to ~ and the delimiters are written #xx (7.3.5).
    // Throws std::invalid_argument when it's empty or holds a zero byte.
    std::string name(std::string_view text);

    // A literal string, "(...)": ( ) and \ escaped, other bytes outside space to ~ as octal (7.3.4.2).
    std::string literal(std::string_view bytes);

    // A hex string, "<...>".
    std::string hex(std::string_view bytes);

    // The code points of UTF-8 text; throws std::invalid_argument when it's malformed.
    std::u32string code_points(std::string_view utf8);

    // A text string (7.9.2.2), from UTF-8: a literal when it's printable ASCII, else UTF-16BE with
    // its byte order mark, in hex. Throws std::invalid_argument on malformed UTF-8.
    std::string text_string(std::string_view utf8);

    // A date (7.9.4): (D:20261019143812+02'00'), in the computer's time zone.
    std::string date(std::chrono::system_clock::time_point time);

    // An indirect reference: "12 0 R".
    std::string reference(std::uint32_t id);

    // The file: a header, objects in any order (each written once, by the number allocate() gave
    // it), and on finish() the cross-reference table and trailer. Objects go to the file as they're
    // written; nothing is held but their offsets.
    class file {
    public:
        // Throws std::runtime_error when the file can't be created.
        explicit file(const std::filesystem::path &path);

        // The next object number, to write later (and refer to before then).
        std::uint32_t allocate();

        // Writes object id: "id 0 obj", body, "endobj".
        void object(std::uint32_t id, std::string_view body);

        // Writes object id as a stream: dictionary holds its entries (without /Length or /Filter),
        // and data is compressed with zlib (FlateDecode) when compress is set.
        void stream(std::uint32_t id, std::string_view dictionary, std::string_view data, bool compress = true);

        // Writes the cross-reference table and the trailer, naming root and info (0: none), and
        // closes the file. Throws std::logic_error when an allocated object wasn't written, and
        // std::runtime_error when writing failed.
        void finish(std::uint32_t root, std::uint32_t info);

        [[nodiscard]] bool finished() const { return finished_; }

    private:
        void write(std::string_view bytes);

        std::filesystem::path path_;
        std::ofstream out_;
        std::uint64_t offset_ = 0;
        std::vector<std::uint64_t> offsets_; // by object number - 1; 0 until written
        bool finished_ = false;
    };

    // zlib's deflate of data, as FlateDecode reads it.
    std::string deflate(std::string_view data);
}

#endif //SC_PDF_ENGINE_OBJECTS_H
