#pragma once

#include <map>
#include <string>

namespace cxx {

    /**
     * @interface IHttpClient
     * @brief Interface for HTTP client implementations.
     *
     * This interface defines the contract for HTTP client implementations,
     * providing methods for standard HTTP operations (GET, POST, PUT, DELETE).
     * It allows for consistent HTTP request handling across different underlying
     * HTTP client implementations.
     */
    class IHttpClient {
    public:
        /**
         * @typedef Headers
         * @brief Type definition for HTTP headers.
         *
         * A map where keys are header names and values are header values.
         */
        using Headers = std::map< std::string, std::string >;

        /**
         * @struct Response
         * @brief Structure representing an HTTP response.
         *
         * Contains all relevant information from an HTTP response including
         * status code, response body, and headers.
         */
        struct Response {
            /** @brief HTTP status code (e.g., 200, 404, 500) */
            int statusCode;

            /** @brief Response body content */
            std::string body;

            /** @brief Response headers as key-value pairs */
            Headers headers;
        };

        /**
         * @brief Virtual destructor to ensure proper cleanup in derived classes.
         */
        virtual ~IHttpClient();

        /**
         * @brief Performs an HTTP GET request.
         *
         * @param url The URL to request.
         * @param headers Additional headers to include with the request.
         * @return Response The HTTP response containing status code, body, and headers.
         * @throw May throw implementation-specific exceptions for network or HTTP errors.
         */
        virtual Response get(const std::string & url, Headers headers = {}) = 0;

        /**
         * @brief Performs an HTTP POST request.
         *
         * @param url The URL to request.
         * @param body The request body to send.
         * @param headers Additional headers to include with the request.
         * @return Response The HTTP response containing status code, body, and headers.
         * @throw May throw implementation-specific exceptions for network or HTTP errors.
         */
        virtual Response post(const std::string & url, const std::string & body, Headers headers = {}) = 0;

        /**
         * @brief Performs an HTTP PUT request.
         *
         * @param url The URL to request.
         * @param body The request body to send.
         * @param headers Additional headers to include with the request.
         * @return Response The HTTP response containing status code, body, and headers.
         * @throw May throw implementation-specific exceptions for network or HTTP errors.
         */
        virtual Response put(const std::string & url, const std::string & body, Headers headers = {}) = 0;

        /**
         * @brief Performs an HTTP DELETE request.
         *
         * @param url The URL to request.
         * @param headers Additional headers to include with the request.
         * @return Response The HTTP response containing status code, body, and headers.
         * @throw May throw implementation-specific exceptions for network or HTTP errors.
         */
        virtual Response del(const std::string & url, Headers headers = {}) = 0;
    };

} // namespace cxx
