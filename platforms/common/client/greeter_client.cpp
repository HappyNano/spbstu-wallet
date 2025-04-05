#include "greeter_client.h"
#include <grpcpp/grpcpp.h>

namespace helloworld {

    GreeterClient::GreeterClient(std::shared_ptr< grpc::Channel > channel)
      : stub_(Greeter::NewStub(channel)) {
    }

    std::string GreeterClient::SayHello(const std::string & name) {
        // Data we are sending to the server.
        HelloRequest request;
        request.set_name(name);

        // Container for the data we expect from the server.
        HelloReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        // The actual RPC.
        grpc::Status status = stub_->SayHello(&context, request, &reply);

        // Act upon its status.
        if (status.ok()) {
            return reply.message();
        } else {
            std::string errorMessage = "RPC failed: " + status.error_message();
            return errorMessage;
        }
    }

} // namespace helloworld
