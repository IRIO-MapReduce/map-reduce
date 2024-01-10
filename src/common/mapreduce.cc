#include <fstream>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"
#include "mapreduce.h"
#include "utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

void map_reduce(Config const& config) {
    std::cerr << "[CLIENT] Starting mapreduce." << std::endl;

    std::string master_address(MASTER_ADDRESS);
    std::shared_ptr<Channel> channel = grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<Master::Stub> masterStub = Master::NewStub(channel);

    ClientRequest request;
    request.set_input_filepath(config.input_filepath);
    request.set_output_filepath(config.output_filepath);
    request.set_mapper_execpath(config.mapper_execpath);
    request.set_reducer_execpath(config.reducer_execpath);
    request.set_split_size_bytes(config.split_size_bytes);
    request.set_num_reducers(config.num_reducers);

    ClientContext context;
    Response response;
    Status status = masterStub->ProcessClientRequest(&context, request, &response);

    assert(status.ok());
}

} // mapreduce