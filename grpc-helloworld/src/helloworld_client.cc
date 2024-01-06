#include <grpc++/grpc++.h>
#include "helloworld.grpc.pb.h"

void RunClient() {
    std::string target("localhost:50051");
    helloworld::Greeter::Stub stub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    helloworld::HelloRequest request;
    helloworld::HelloReply reply;

    request.set_name("World");
    grpc::ClientContext context;

    grpc::Status status = stub.SayHello(&context, request, &reply);

    if (status.ok()) {
        std::cout << "Received reply: " << reply.message() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
}

int main() {
    RunClient();
    return 0;
}