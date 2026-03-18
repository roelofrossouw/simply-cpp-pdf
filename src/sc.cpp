#include "sc.h"

namespace sc {
    std::string file_get_contents(const std::string &filename) {
        std::stringstream stream;
        std::ifstream input_file(filename.data());
        input_file >> stream.rdbuf();
        return stream.str();
    }

    std::string basename(const std::string &filename) {
        std::filesystem::path app_name(filename);
        return app_name.filename();
    }

    void file_put_contents(const std::string &filename, const std::string &content, const std::ios_base::openmode mode) {
        std::ofstream output_file(filename, mode);
        output_file << content;
        output_file.close();
    }
}
