#include <svg2png.h>
#include <lunasvg.h>
#include <stdexcept>

using namespace lunasvg;
using namespace std;

namespace sc {
    void svg2png::copy_data(void *target, void *source, int size) {
        auto *png_string = (string *) target;
        *png_string = string(static_cast<char *>(source), size);
    }

    std::string svg2png::PngData(void *data) {
        if (data == nullptr) throw std::invalid_argument("svg2png::PngData: Invalid SVG data");
        Document *document = static_cast<Document *>(data);
        auto bitmap = document->renderToBitmap();
        if (bitmap.isNull()) throw std::invalid_argument("svg2png::PngData: Could not create PNG");
        string png_data;
        bitmap.writeToPng(copy_data, &png_data);
        return png_data;
    }

    std::string svg2png::FromString(const std::string &svg_data) {
        auto document = Document::loadFromData(svg_data.c_str(), svg_data.size());
        return PngData(document.get());
    }

    std::string svg2png::FromFile(const std::string &filename) {
        auto document = Document::loadFromFile(filename);
        return PngData(document.get());
    }
}
