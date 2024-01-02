#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloWorld;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class ReducerClient {
public:
    ReducerClient(std::shared_ptr<Channel> channel)
        : stub_(HelloWorld::NewStub(channel)) {}

    std::string SayHello(const std::string& user) {
        HelloRequest request;
        request.set_name(user);

        HelloReply reply;
        ClientContext context;
        Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok()) {
            return reply.message();
        } 
        else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<HelloWorld::Stub> stub_;
};

int main() {
    std::string server_address("0.0.0.0:50051");
    ReducerClient reducer(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    std::string user("reducer");
    std::string reply = reducer.SayHello(user);
    std::cout << "Reducer received: " << reply << std::endl;

    return 0;
}
