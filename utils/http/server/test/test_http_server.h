#pragma once

#include <atomic>
#include <cstring>
#include <ctime>
#include <map>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define SOCKET      int
#define closesocket close
#endif

namespace cxx {

    /**
     * @class TestHttpServer
     * @brief A simple HTTP server for testing HTTP clients.
     *
     * This class implements a lightweight HTTP server that can be used in unit tests
     * to verify HTTP client behavior without making real network requests. It allows
     * setting predefined responses and inspecting received requests.
     *
     * The server runs in a separate thread and can handle basic HTTP requests.
     * It supports cross-platform operation on both Windows and POSIX systems.
     */
    class TestHttpServer final {
    public:
        /**
         * @typedef Headers
         * @brief Type definition for HTTP headers.
         *
         * A map where keys are header names and values are header values.
         */
        using Headers = std::map< std::string, std::string >;

        /**
         * @brief Constructs a TestHttpServer listening on the specified port.
         *
         * @param port The TCP port to listen on (default: 8080).
         */
        explicit TestHttpServer(int port = 8080);

        /**
         * @brief Destructor that stops the server and cleans up resources.
         *
         * Stops the server thread and performs necessary cleanup for socket resources.
         */
        ~TestHttpServer();

        /**
         * @brief Copy constructor (deleted).
         *
         * TestHttpServer is not copyable due to thread and socket management.
         */
        TestHttpServer(const TestHttpServer &) = delete;

        /**
         * @brief Copy assignment operator (deleted).
         *
         * TestHttpServer is not copyable due to thread and socket management.
         */
        TestHttpServer & operator=(const TestHttpServer &) = delete;

        // Server management methods

        /**
         * @brief Gets the base URL of the server.
         *
         * @return std::string The base URL in the format "http://localhost:port".
         */
        auto getBaseUrl() const -> std::string;

        /**
         * @brief Sets the response that the server will send for subsequent requests.
         *
         * @param statusCode The HTTP status code to return.
         * @param body The response body content.
         * @param headers Additional headers to include in the response.
         */
        void setResponse(int statusCode, const std::string & body, const Headers & headers = {});

        // Request inspection methods

        /**
         * @brief Checks if the server has received any requests.
         *
         * @return bool True if a request has been received, false otherwise.
         */
        auto wasRequestReceived() const -> bool;

        /**
         * @brief Gets the HTTP method from the last received request.
         *
         * @return const std::string& The HTTP method (GET, POST, PUT, DELETE, etc.).
         */
        auto getLastRequestMethod() const -> const std::string &;

        /**
         * @brief Gets the path from the last received request.
         *
         * @return const std::string& The request path.
         */
        auto getLastRequestPath() const -> const std::string &;

        /**
         * @brief Gets the body content from the last received request.
         *
         * @return const std::string& The request body.
         */
        auto getLastRequestBody() const -> const std::string &;

        /**
         * @brief Gets the headers from the last received request.
         *
         * @return Headers A map of header names to values.
         */
        auto getLastRequestHeaders() const -> Headers;

    private:
        /**
         * @brief Main server loop that runs in a separate thread.
         *
         * Listens for incoming connections and handles HTTP requests.
         */
        void run();

        /**
         * @brief Handles a single client connection.
         *
         * Reads the request, parses it, and sends the configured response.
         *
         * @param sockfd The socket file descriptor for the client connection.
         */
        void handleConnection(SOCKET sockfd);

        /**
         * @brief Parses an HTTP request string.
         *
         * Extracts method, path, headers, and body from the request.
         *
         * @param request The raw HTTP request string.
         */
        void parseRequest(const std::string & request);

        /**
         * @brief Builds an HTTP response string.
         *
         * Constructs a properly formatted HTTP response using the configured
         * status code, headers, and body.
         *
         * @return std::string The formatted HTTP response.
         */
        std::string buildResponse();

#ifdef _WIN32
        /**
         * @brief Initializes Winsock on Windows platforms.
         */
        void initWinsock();

        /**
         * @brief Cleans up Winsock resources on Windows platforms.
         */
        void cleanupWinsock();
#endif

    private:
        /** @brief The TCP port the server listens on. */
        int port_;

        /** @brief Flag indicating whether the server is running. */
        std::atomic< bool > running_{ false };

        /** @brief Thread for the server's main loop. */
        std::thread serverThread_;

        /** @brief HTTP status code to include in responses. */
        int statusCode_{ 200 };

        /** @brief Body content to include in responses. */
        std::string responseBody_{ "OK" };

        /** @brief Headers to include in responses. */
        Headers responseHeaders_;

        /** @brief Flag indicating whether a request has been received. */
        std::atomic< bool > requestReceived_{ false };

        /** @brief HTTP method from the last received request. */
        std::string lastRequestMethod_;

        /** @brief Path from the last received request. */
        std::string lastRequestPath_;

        /** @brief Body content from the last received request. */
        std::string lastRequestBody_;

        /** @brief Headers from the last received request. */
        Headers lastRequestHeaders_;

#ifdef _WIN32
        /** @brief Flag indicating whether Winsock has been initialized. */
        bool winsockInitialized_{ false };
#endif
    };

} // namespace cxx
