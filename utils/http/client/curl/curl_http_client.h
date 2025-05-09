#pragma once

#include <utils/http/client/interface/i_http_client.h>

#include <curl/curl.h>

#include <string>

namespace cxx {

    /**
     * @class CurlHttpClient
     * @brief HTTP client implementation using libcurl.
     *
     * This class provides HTTP client functionality by implementing the IHttpClient
     * interface using the libcurl library. It supports common HTTP methods (GET, POST,
     * PUT, DELETE) and allows customization of request headers and connection settings.
     */
    class CurlHttpClient final: public IHttpClient {
    public:
        /**
         * @struct Settings
         * @brief Configuration settings for the HTTP client.
         *
         * This structure contains various settings to configure the behavior
         * of the HTTP client, such as timeouts and SSL verification.
         */
        struct Settings {
            /** @brief Whether to follow HTTP redirects. Default is true. */
            bool followRedirects = true;

            /** @brief Whether to verify SSL certificates. Default is true. */
            bool verifySSL = true;

            /** @brief Connection timeout in seconds. Default is 10 seconds. */
            long connectTimeout = 10;

            /** @brief Operation timeout in seconds. Default is 30 seconds. */
            long timeout = 30;

            /** @brief Enable verbose output for debugging. Default is false. */
            bool verbose = false;
        };

        /**
         * @brief Constructs a CurlHttpClient
         *
         * @param settings Configuration settings for the HTTP client.
         * @param initialHeaders Headers to be included in every request.
         * @throw std::runtime_error If curl initialization fails.
         */
        explicit CurlHttpClient(
         Settings settings,
         Headers initialHeaders = {});

        ~CurlHttpClient() override;

        /**
         * @brief Copy constructor (deleted).
         *
         * Copying is not allowed because this class manages a CURL handle.
         */
        CurlHttpClient(const CurlHttpClient &) = delete;

        /**
         * @brief Copy assignment operator (deleted).
         *
         * Copying is not allowed because this class manages a CURL handle.
         */
        CurlHttpClient & operator=(const CurlHttpClient &) = delete;

        // IHttpClient interface implementation

        /**
         * @brief Performs an HTTP GET request.
         *
         * @param url The URL to request.
         * @param headers Additional headers to include in this request.
         * @return Response The HTTP response with status code, body, and headers.
         * @throw std::runtime_error If the request fails.
         */
        Response get(const std::string & url, Headers headers = {}) override;

        /**
         * @brief Performs an HTTP POST request.
         *
         * @param url The URL to request.
         * @param body The request body to send.
         * @param headers Additional headers to include in this request.
         * @return Response The HTTP response with status code, body, and headers.
         * @throw std::runtime_error If the request fails.
         */
        Response post(const std::string & url, const std::string & body, Headers headers = {}) override;

        /**
         * @brief Performs an HTTP PUT request.
         *
         * @param url The URL to request.
         * @param body The request body to send.
         * @param headers Additional headers to include in this request.
         * @return Response The HTTP response with status code, body, and headers.
         * @throw std::runtime_error If the request fails.
         */
        Response put(const std::string & url, const std::string & body, Headers headers = {}) override;

        /**
         * @brief Performs an HTTP DELETE request.
         *
         * @param url The URL to request.
         * @param headers Additional headers to include in this request.
         * @return Response The HTTP response with status code, body, and headers.
         * @throw std::runtime_error If the request fails.
         */
        Response del(const std::string & url, Headers headers = {}) override;

    private:
        /** @brief Client configuration settings. */
        const Settings settings_;

        /** @brief Headers to be included in every request. */
        const Headers initialHeaders_;

        /** @brief The CURL handle used for HTTP operations. */
        CURL * curl_;

        /**
         * @brief Internal method to perform HTTP requests.
         *
         * This method is used by the public HTTP method functions to perform
         * the actual HTTP request using libcurl.
         *
         * @param url The URL to request.
         * @param method The HTTP method (GET, POST, PUT, DELETE).
         * @param body The request body (for POST and PUT requests).
         * @param requestHeaders Additional headers to include in this request.
         * @return Response The HTTP response with status code, body, and headers.
         * @throw std::runtime_error If the request fails.
         */
        Response performRequest(const std::string & url, const std::string & method, const std::string & body = "", Headers requestHeaders = {});
    };

} // namespace cxx
