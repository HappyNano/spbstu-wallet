#include "curl_http_client.h"
#include "utils/http/client/interface/i_http_client.h"

#include <utils/string/trim.h>

#include <mutex>
#include <stdexcept>

using namespace cxx;

namespace {

    const IHttpClient::Headers DEFAULT_HEADERS = {
        { "User-Agent", "CurlHttpClient/1.0" }
    };

    size_t writeCallback(void * contents, size_t size, size_t nmemb, std::string * userp) {
        userp->append(static_cast< char * >(contents), size * nmemb);
        return size * nmemb;
    }

    size_t headerCallback(char * buffer, size_t size, size_t nitems, IHttpClient::Headers * headers) {
        size_t headerSize = size * nitems;
        std::string header(buffer, headerSize);

        // Remove trailing \r\n
        if (header.size() >= 2 && header.substr(header.size() - 2) == "\r\n") {
            header = header.substr(0, header.size() - 2);
        }

        // Skip empty headers
        if (header.empty()) {
            return headerSize;
        }

        // Find the colon separator
        size_t colonPos = header.find(':');
        if (colonPos != std::string::npos) {
            std::string name = header.substr(0, colonPos);
            std::string value = header.substr(colonPos + 1);

            // Trim leading/trailing whitespace
            ltrim(value);
            rtrim(name);
            rtrim(value);

            (*headers)[name] = value;
        }

        return headerSize;
    }

    CURL * makeCurlHandler() {
        static std::once_flag curlInitializationFlag;
        std::call_once(curlInitializationFlag, [] { curl_global_init(CURL_GLOBAL_ALL); });
        CURL * handler = curl_easy_init();
        if (!handler) {
            throw std::runtime_error("Failed to initialize curl");
        }
        return handler;
    }

    IHttpClient::Headers mergeHeaders(const IHttpClient::Headers & init, IHttpClient::Headers over) {
        over.insert(init.begin(), init.end());
        return over;
    }

    struct curl_slist * headersToCurlList(const IHttpClient::Headers & headers) {
        struct curl_slist * curlHeaders = nullptr;

        for (const auto & header: headers) {
            std::string headerStr = header.first + ": " + header.second;
            curlHeaders = curl_slist_append(curlHeaders, headerStr.c_str());
        }

        return curlHeaders;
    }

} // unnamed namespace

CurlHttpClient::CurlHttpClient(Settings settings, Headers initialHeaders)
  : settings_(std::move(settings))
  , initialHeaders_(mergeHeaders(DEFAULT_HEADERS, std::move(initialHeaders)))
  , curl_(makeCurlHandler()) {
}

CurlHttpClient::~CurlHttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

IHttpClient::Response CurlHttpClient::get(const std::string & url, Headers headers) {
    return performRequest(url, "GET", "", headers);
}

IHttpClient::Response CurlHttpClient::post(const std::string & url, const std::string & body, Headers headers) {
    return performRequest(url, "POST", body, headers);
}

IHttpClient::Response CurlHttpClient::put(const std::string & url, const std::string & body, Headers headers) {
    return performRequest(url, "PUT", body, headers);
}

IHttpClient::Response CurlHttpClient::del(const std::string & url, Headers headers) {
    return performRequest(url, "DELETE", "", headers);
}

IHttpClient::Response CurlHttpClient::performRequest(const std::string & url, const std::string & method, const std::string & body, Headers requestHeaders) {
    Response response;
    std::string responseBody;

    curl_easy_reset(curl_);

    // Set URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    // Set write callback for body
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &responseBody);

    // Set write callback for headers
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response.headers);

    // Create curl headers list (request headers over client headers)
    struct curl_slist * localHeaders = headersToCurlList(mergeHeaders(initialHeaders_, std::move(requestHeaders)));
    if (localHeaders) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, localHeaders);
    }

    // Set request method
    if (method != "GET") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, method.c_str());
    }

    // Set request body for POST and PUT requests
    if ((method == "POST" || method == "PUT") && !body.empty()) {
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());
    }

    // Set options from settings
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, settings_.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, settings_.verifySSL ? 1L : 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, settings_.verifySSL ? 2L : 0L);
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, settings_.connectTimeout);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, settings_.timeout);
    curl_easy_setopt(curl_, CURLOPT_VERBOSE, settings_.verbose ? 1L : 0L);

    // Perform request
    CURLcode res = curl_easy_perform(curl_);

    // Clean up temporary headers
    if (localHeaders) {
        curl_slist_free_all(localHeaders);
    }

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res));
    }

    // Get status code
    long statusCode;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &statusCode);

    response.statusCode = static_cast< int >(statusCode);
    response.body = responseBody;

    return response;
}
