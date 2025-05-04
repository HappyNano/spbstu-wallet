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
    // Test HTTP server for simulating responses in client tests
    class TestHttpServer final {
    public:
        using Headers = std::map< std::string, std::string >;

        explicit TestHttpServer(int port = 8080);
        ~TestHttpServer();

        // Non-copyable
        TestHttpServer(const TestHttpServer &) = delete;
        TestHttpServer & operator=(const TestHttpServer &) = delete;

        // Server management methods
        auto getBaseUrl() const -> std::string;
        void setResponse(int statusCode, const std::string & body, const Headers & headers = {});

        // Request inspection methods
        auto wasRequestReceived() const -> bool;
        auto getLastRequestMethod() const -> const std::string &;
        auto getLastRequestPath() const -> const std::string &;
        auto getLastRequestBody() const -> const std::string &;
        auto getLastRequestHeaders() const -> Headers;

    private:
        void run();
        void handleConnection(SOCKET sockfd);
        void parseRequest(const std::string & request);
        std::string buildResponse();
#ifdef _WIN32
        void initWinsock();
        void cleanupWinsock();
#endif

    private:
        int port_;
        std::atomic< bool > running_{ false };
        std::thread serverThread_;

        int statusCode_{ 200 };
        std::string responseBody_{ "OK" };
        Headers responseHeaders_;

        std::atomic< bool > requestReceived_{ false };
        std::string lastRequestMethod_;
        std::string lastRequestPath_;
        std::string lastRequestBody_;
        Headers lastRequestHeaders_;

#ifdef _WIN32
        bool winsockInitialized_{ false };
#endif
    };
} // namespace cxx
