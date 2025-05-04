#include "test_http_server.h"

#include <spdlog/spdlog.h>

using namespace cxx;

namespace {

    std::string getStatusText(int code) {
        switch (code) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown";
        }
    }

} // unnamed namespace

TestHttpServer::TestHttpServer(int port)
  : port_(port) {
#ifdef _WIN32
    initWinsock();
#endif

    // Start server in a separate thread
    serverThread_ = std::thread(&TestHttpServer::run, this);

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TestHttpServer::~TestHttpServer() {
    running_ = false;
    if (serverThread_.joinable()) {
        serverThread_.join();
    }

#ifdef _WIN32
    cleanupWinsock();
#endif
}

auto TestHttpServer::getBaseUrl() const -> std::string {
    return "http://localhost:" + std::to_string(port_);
}

void TestHttpServer::setResponse(int statusCode, const std::string & body, const Headers & headers) {
    statusCode_ = statusCode;
    responseBody_ = body;
    responseHeaders_ = headers;
}

auto TestHttpServer::wasRequestReceived() const -> bool {
    return requestReceived_;
}

auto TestHttpServer::getLastRequestMethod() const -> const std::string & {
    return lastRequestMethod_;
}

auto TestHttpServer::getLastRequestPath() const -> const std::string & {
    return lastRequestPath_;
}

auto TestHttpServer::getLastRequestBody() const -> const std::string & {
    return lastRequestBody_;
}

TestHttpServer::Headers TestHttpServer::getLastRequestHeaders() const {
    return lastRequestHeaders_;
}

#ifdef _WIN32
void TestHttpServer::initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        SPDLOG_ERROR("WSAStartup failed: {}", result);
        return;
    }
    winsockInitialized_ = true;
}

void TestHttpServer::cleanupWinsock() {
    if (winsockInitialized_) {
        WSACleanup();
        winsockInitialized_ = false;
    }
}
#endif

void TestHttpServer::run() {
    // Create socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        SPDLOG_ERROR("Error opening socket");
        return;
    }

    // Set socket option to allow reuse
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    // Bind to port
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        SPDLOG_ERROR("Error on binding");
        closesocket(sockfd);
        return;
    }

    // Listen for connections
    listen(sockfd, 5);
    running_ = true;

    while (running_) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout

        int ret = select(sockfd + 1, &rfds, nullptr, nullptr, &tv);
        if (ret <= 0) {
            continue;
        }

        if (FD_ISSET(sockfd, &rfds)) {
            struct sockaddr_in cliAddr;
            socklen_t clilen = sizeof(cliAddr);
            SOCKET newsockfd = accept(sockfd, (struct sockaddr *)&cliAddr, &clilen);

            if (newsockfd < 0) {
                SPDLOG_ERROR("Error on accept");
                continue;
            }

            // Handle connection
            handleConnection(newsockfd);
            closesocket(newsockfd);
        }
    }

    closesocket(sockfd);
}

void TestHttpServer::handleConnection(SOCKET sockfd) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        return;
    }

    // Parse request
    parseRequest(std::string(buffer));

    // Send response
    std::string response = buildResponse();
    send(sockfd, response.c_str(), static_cast< int >(response.length()), 0);
}

void TestHttpServer::parseRequest(const std::string & request) {
    requestReceived_ = true;

    // Parse method and path
    std::istringstream iss(request);
    std::string line;
    std::getline(iss, line);
    std::istringstream methodLine(line);
    methodLine >> lastRequestMethod_ >> lastRequestPath_;

    // Parse headers
    lastRequestHeaders_.clear();
    while (std::getline(iss, line) && !line.empty() && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string name = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            lastRequestHeaders_[name] = value;
        }
    }

    // Parse body
    lastRequestBody_.clear();
    while (std::getline(iss, line)) {
        lastRequestBody_ += line + "\n";
    }

    if (!lastRequestBody_.empty()) {
        lastRequestBody_.pop_back(); // Remove trailing newline
    }
}

std::string TestHttpServer::buildResponse() {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode_ << " " << getStatusText(statusCode_) << "\r\n";

    // Add Date header
    char timeBuffer[80];
    time_t now = time(nullptr);
    struct tm * timeinfo = gmtime(&now);
    strftime(timeBuffer, sizeof(timeBuffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    oss << "Date: " << timeBuffer << "\r\n";

    // Add Server header
    oss << "Server: TestHttpServer/1.0\r\n";

    // Add Content-Length
    oss << "Content-Length: " << responseBody_.length() << "\r\n";

    // Add custom headers
    for (const auto & header: responseHeaders_) {
        oss << header.first << ": " << header.second << "\r\n";
    }

    // End headers
    oss << "\r\n";

    // Add body
    oss << responseBody_;

    return oss.str();
}
