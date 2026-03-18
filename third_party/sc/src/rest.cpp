#include <rest.h>
#include <sc.h>
#include <curl/curl.h>

namespace sc {
    size_t write_data(void *contents, size_t size, size_t nmemb, void *userp) {
        static_cast<std::string *>(userp)->append((char *) contents, size * nmemb);
        return size * nmemb;
    }

    std::map<std::string, std::string> rest::fetch_cache;

    rest::rest(const std::string &url) : url_(url) {
        headers_["Content-Type"] = "application/json";
    }

    std::string rest::get() {
        CURL *curl = curl_easy_init();
        if (!curl) {
            response = "";
            return response;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());

        curl_slist *headers = nullptr;
        for (auto &hdr: headers_) {
            headers = curl_slist_append(headers, (hdr.first + ": " + hdr.second).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return response;
    }


    std::string rest::post(const char *jsonData) {
        CURL *curl = curl_easy_init();
        if (!curl) {
            response = "";
            return response;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);

        curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return response;
    }

    void rest::header(const std::string &key, const std::string &value) {
        headers_[key] = value;
    }

    void rest::bearer(const std::string &token) {
        header("Authorization", "Bearer " + token);
    }

    std::string rest::fetch(const std::string &url, const std::string &base_url) {
        if (url.size() > 100) return "";
        if (fetch_cache.find(url) != fetch_cache.end()) {
            return fetch_cache[url];
        }

        std::string content;

        if (std::filesystem::exists(url)) {
            content = file_get_contents(url);
        }

        if (content.empty() && std::filesystem::exists(url.substr(1))) {
            content = file_get_contents(url.substr(1));
        }

        if (content.empty() && url.substr(0, base_url.length()) == base_url) {
            auto local_path = url.substr(base_url.size());
            if (std::filesystem::exists(local_path)) {
                content = file_get_contents(local_path);
            }
        }

        if (content.empty()) {
            rest rest(url);
            content = rest.get();
        }

        fetch_cache[url] = content;
        return fetch_cache[url];

        // rest rest(url);
        // fetch_cache[url] = rest.get();
        // return fetch_cache[url];
    }
}
