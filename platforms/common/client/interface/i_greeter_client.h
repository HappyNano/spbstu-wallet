#pragma once

#include <string>

namespace helloworld {
    class IGreeterClient {
    public:
        virtual ~IGreeterClient() = default;
        virtual std::string SayHello(const std::string & name) = 0;
    };
} // namespace helloworld
