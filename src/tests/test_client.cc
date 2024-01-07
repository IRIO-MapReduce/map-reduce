#include <grpc++/grpc++.h>
#include "mapreduce.grpc.pb.h"

void RunClient() {
    std::string target("localhost:50051");
    mapreduce::Server::Stub stub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    mapreduce::Request request;
    mapreduce::Reply reply;

    request.set_filename("input.txt");
    grpc::ClientContext context;

    grpc::Status status = stub.MapReduce(&context, request, &reply);

    if (status.ok()) {
        std::cout << "Received reply: " << reply.filename() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
}

int main() {
    RunClient();
    return 0;
}