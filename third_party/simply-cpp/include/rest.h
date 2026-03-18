#ifndef REST_H
#define REST_H

#include <string>
#include <map>

namespace sc {
    class rest {
    public:
        explicit rest(const std::string &url);

        std::string get();

        std::string post(const char *jsonData = R"({})");

        void header(const std::string &key, const std::string &value);

        void bearer(const std::string &token);

        static std::string fetch(const std::string &url, const std::string &base_url = "");

    private:
        std::string url_;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> headers_;
        std::string response;

        static std::map<std::string, std::string> fetch_cache;
    };
}

#endif //REST_H
