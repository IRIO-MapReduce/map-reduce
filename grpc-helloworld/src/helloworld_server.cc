#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include "helloworld.grpc.pb.h"

class GreeterServiceImpl final : public helloworld::Greeter::Service {
public:
    grpc::Status SayHello(grpc::ServerContext* context, const helloworld::HelloRequest* request, helloworld::HelloReply* reply) override {
        std::string prefix("Hello, ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}