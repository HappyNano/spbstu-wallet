#pragma once

#include <map>
#include <string>

namespace cxx {

    class IHttpClient {
    public:
        using Headers = std::map< std::string, std::string >;

        struct Response {
            int statusCode;
            std::string body;
            Headers headers;
        };

        virtual ~IHttpClient();

        virtual Response get(const std::string & url, Headers headers = {}) = 0;
        virtual Response post(const std::string & url, const std::string & body, Headers headers = {}) = 0;
        virtual Response put(const std::string & url, const std::string & body, Headers headers = {}) = 0;
        virtual Response del(const std::string & url, Headers headers = {}) = 0;
    };

} // namespace cxx
