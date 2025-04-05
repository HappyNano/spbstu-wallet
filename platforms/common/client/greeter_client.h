#pragma once

#include "platforms/common/client/interface/i_greeter_client.h"
#include <build/proto/helloworld/helloworld.grpc.pb.h>
#include <grpcpp/grpcpp.h>

#include <platforms/common/client/interface/i_greeter_client.h>

#include <memory>
#include <string>

namespace helloworld {
    class GreeterClient: public IGreeterClient {
    public:
        GreeterClient(std::shared_ptr< grpc::Channel > channel);
        ~GreeterClient() override;

        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        std::string SayHello(const std::string & name) override;

    private:
        std::unique_ptr< Greeter::Stub > stub_;
    };
} // namespace helloworld
