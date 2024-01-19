#include <fstream>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"
#include "mapreduce.h"
#include "utils.h"
#include "cloud-utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

void map_reduce(Config const& config) {
    std::cerr << "[CLIENT] Starting mapreduce." << std::endl;

    std::string master_address(get_address(get_master_ip().value(), MASTER_PORT));
    std::shared_ptr<Channel> channel = grpc::CreateChannel(master_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<Master::Stub> masterStub = Master::NewStub(channel);

    validate_executable(config.mapper_execpath);
    validate_executable(config.reducer_execpath);
    
    if (!has_valid_format(config.input_filepath)) {
        throw std::invalid_argument("invalid input_filepath");
    }

    if (config.num_reducers == 0) {
        throw std::invalid_argument("num_reducers must be greater than 0");
    }

    size_t num_mappers = split_file_bytes(config.input_filepath, config.split_size_bytes);

    ClientRequest request;
    request.set_input_filepath(config.input_filepath);
    request.set_output_filepath(config.output_filepath);
    request.set_mapper_execpath(config.mapper_execpath);
    request.set_reducer_execpath(config.reducer_execpath);
    request.set_num_mappers(num_mappers);
    request.set_num_reducers(config.num_reducers);

    ClientContext context;
    Response response;
    Status status = masterStub->ProcessClientRequest(&context, request, &response);

    assert(status.ok());
}

} // mapreduce