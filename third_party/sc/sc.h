#ifndef SC_H
#define SC_H

// Include common standard libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

// Include common libraries
#include <include/base64.h>
#include <include/svg2png.h>
#include <include/rest.h>
#include <include/timer.h>

// Some common functions simplifying STL
namespace sc {
    std::string file_get_contents(const std::string &filename);

    std::string basename(const std::string &filename);

    void file_put_contents(const std::string &filename, const std::string &content, std::ios_base::openmode mode = std::ios_base::binary);
}


#endif //SC_H
