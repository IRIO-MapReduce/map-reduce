#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <grpc++/grpc++.h>
#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;
using grpc::ClientAsyncReader;
using grpc::ClientAsyncReaderWriter;
using grpc::ClientAsyncWriter;
using grpc::ClientAsyncResponseReaderInterface;
using grpc::ServerReader;
using grpc::ServerWriter;

using namespace mapreduce;

class ReducerListenerServiceImpl final : public ReducerListener::Service {
public:
    Status ProcessReduceRequest(ServerContext* context, ServerReader<ReduceRequest>* reader, Response* response) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceRequest." << std::endl;
        
        std::vector<ReduceConfig> configs;
        ReduceRequest request;

        int iter = 0;
        while (reader->Read(&request)) {
            ReduceConfig config;
            config.set_id(request.id());
            config.set_input_filepath(request.input_filepath());
            config.set_output_filepath(request.output_filepath());
            configs.push_back(config);
            std::cerr << " ----------------  filepath " << iter++ << ": " << request.input_filepath() << std::endl;
        }

        std::cerr << " ----------------  execpath: " << request.execpath() << std::endl;
        std::cerr << " ----------------  output_filepath: " << request.output_filepath() << std::endl;

        // Save config for later.
        work_queue.add(request.execpath(), configs);

        // Run reducer binary
        std::system(std::string(request.execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetReduceConfig(ServerContext* context, const ReduceConfigRequest* request, ServerWriter<ReduceConfig>* writer) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceConfig request." << std::endl;
        
        // Respond with saved config.
        std::vector<ReduceConfig> configs = work_queue.get_one(request->execpath());

        for (auto const& config : configs) {
            writer->Write(config);
        }

        return Status::OK;
    }

private:
    ListenerWorkQueue<std::vector<ReduceConfig>> work_queue;
};

void RunReducerListenerServer() {
    std::string server_address(REDUCER_LISTENER_ADDRESS);
    ReducerListenerServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cerr << "[REDUCER LISTENER] Server listening on " << server_address << std::endl;
    
    server->Wait();
}

int main() {
    RunReducerListenerServer();
    return 0;
}