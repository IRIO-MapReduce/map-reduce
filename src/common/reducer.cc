#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "reducer.h"
#include "utils.h"
#include "mapreduce.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

bool Reducer::get_next_pair(key_t& key, val_t& val) {
    if (!input_file.is_open()) {
        if (input_filepaths.empty()) {
            return false;
        }

        try {
            input_file.open(input_filepaths.back());
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

    input_file.close();
    input_filepaths.pop_back();

    return get_next_pair(key, val);
}

void Reducer::emit(key_t const& key, val_t const& val) {
    std::ofstream output_file(output_filepath, std::ios::app);
    output_file << key + "," + val + "\n";  
}

void Reducer::start() {
    std::cerr << "[REDUCER WORKER] Starting worker..." << std::endl;

    std::string reducer_listener_address(REDUCER_LISTENER_ADDRESS);
    std::shared_ptr<Channel> channel = grpc::CreateChannel(reducer_listener_address, grpc::InsecureChannelCredentials());
    std::unique_ptr<ReducerListener::Stub> stub = ReducerListener::NewStub(channel);

    ReduceConfigRequest request;
    request.set_nothing("nothing");

    ClientContext context;
    ReduceConfig config;

    std::cerr << "[MAPPER WORKER] Requesting MapConfig from listener" << std::endl;

    std::unique_ptr<grpc::ClientReader<ReduceConfig>> reader(
        stub->GetReduceConfig(&context, request));

    
    while (reader->Read(&config)) {
        std::cerr << "[REDUCER WORKER] Received ReduceConfig from listener" << std::endl;
        std::cerr << " ---------------  input_filepath: " << config.input_filepath() << std::endl;
        std::cerr << " ---------------  output_filepath: " << config.output_filepath() << std::endl;

        this->input_filepaths.push_back(config.input_filepath());
        this->output_filepath = config.output_filepath();
    }

    std::cerr << "[REDUCER WORKER] Starting reduce()" << std::endl;

    reduce();
        
    std::cerr << "[REDUCER WORKER] Finished reduce()" << std::endl;

    Status status = reader->Finish();
    assert(status.ok());

    std::cerr << "[REDUCER WORKER] Finished all work" << std::endl;
}
    

} // mapreduce