#ifndef SC_BASE64_H
#define SC_BASE64_H
#include <string>

namespace sc {
    class base64 {
    public:
        static std::string decode(const std::string &encoded);

        static std::string encode(const std::string &text);
    };
}

#endif //SC_BASE64_H
