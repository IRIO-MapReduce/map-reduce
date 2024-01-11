#include <iostream>
#include <fstream>
#include <sstream>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "mapper.h"
#include "utils.h"
#include "mapreduce.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

bool Mapper::get_next_pair(key_t& key, val_t& val) {
    if (!input_file.is_open()) {
        try {
            input_file.open(input_filepath);
        }
        catch (...) {
            std::cerr << "[ERROR] Error opening file" << std::endl;
            return false;
        }
    }

    std::string line;
    if (getline(input_file, line)) {
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, val);
        return true;
    }

    return false;
}

void Mapper::emit(key_t const& key, val_t const& val) {
    size_t h = std::hash<key_t>()(key) % num_reducers;
    std::string filepath = get_intermediate_filepath(input_filepath, h);
    std::ofstream output_file(filepath, std::ios::app);
    output_file << key + "," + val + "\n";
}

void Mapper::start() {
    std::cerr << "[MAPPER WORKER] Starting worker..." << std::endl;

    std::string mapper_listener_address(MAPPER_LISTENER_ADDRESS);
    std::shared_ptr<Channel> channel = grpc::CreateChannel(mapper_listener_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<MapperListener::Stub> stub = MapperListener::NewStub(channel);

    MapConfigRequest request;
    request.set_nothing("nothing");

    ClientContext context;
    MapConfig config;

    std::cerr << "[MAPPER WORKER] Requesting MapConfig from listener" << std::endl;
    
    Status status = stub->GetMapConfig(&context, request, &config);

    assert(status.ok());

    this->input_filepath = config.filepath();
    this->num_reducers = config.num_reducers();

    std::cerr << "[MAPPER WORKER] Worker info retrieved, starting map..." << std::endl;

    map();
    
    std::cerr << "[MAPPER WORKER] Map completed!" << std::endl;
}

} // mapreduce