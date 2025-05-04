#pragma once

#include <utils/http/client/interface/i_http_client.h>

#include <curl/curl.h>

#include <string>

namespace cxx {

    class CurlHttpClient final: public IHttpClient {
    public:
        struct Settings {
            bool followRedirects = true;
            bool verifySSL = true;
            long connectTimeout = 10;
            long timeout = 30;
            bool verbose = false;
        };

        explicit CurlHttpClient(
         Settings settings,
         Headers initialHeaders = {});
        ~CurlHttpClient() override;

        CurlHttpClient(const CurlHttpClient &) = delete;
        CurlHttpClient & operator=(const CurlHttpClient &) = delete;

        // IHttpClient
        Response get(const std::string & url, Headers headers = {}) override;
        Response post(const std::string & url, const std::string & body, Headers headers = {}) override;
        Response put(const std::string & url, const std::string & body, Headers headers = {}) override;
        Response del(const std::string & url, Headers headers = {}) override;

    private:
        const Settings settings_;
        const Headers initialHeaders_;
        CURL * curl_;

        Response performRequest(const std::string & url, const std::string & method, const std::string & body = "", Headers requestHeaders = {});
    };

} // namespace cxx
