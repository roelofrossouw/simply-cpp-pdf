#include "base64.h"
#if __cplusplus >= 202002L
typedef ssize_t rsize_t;
#endif
#include <base64.hpp>

namespace sc {
    std::string base64::decode(const std::string &encoded) {
        return ::base64::from_base64(encoded);
    }

    std::string base64::encode(const std::string &text) {
        return ::base64::to_base64(text);
    }
}
