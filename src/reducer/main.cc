#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <grpc++/grpc++.h>
#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"

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

std::vector<std::string> last_input_filepaths;
std::string last_execpath;
std::string last_output_filepath;

class ReducerListenerServiceImpl final : public ReducerListener::Service {
public:
    Status ProcessReduceRequest(ServerContext* context, ServerReader<ReduceRequest>* reader, Response* response) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceRequest." << std::endl;
        
        last_input_filepaths.clear();

        ReduceRequest request;
        int iter = 0;
        while (reader->Read(&request)) {
            std::cerr << " ----------------  filepath " << iter++ << ": " << request.input_filepath() << std::endl;
            last_input_filepaths.push_back(request.input_filepath());
        }

        std::cerr << " ----------------  execpath: " << request.execpath() << std::endl;
        std::cerr << " ----------------  output_filepath: " << request.output_filepath() << std::endl;

        last_execpath = request.execpath();
        last_output_filepath = request.output_filepath();

        // Run mapper binary
        std::system(request.execpath().c_str());

        return Status::OK;
    }

    Status GetReduceConfig(ServerContext* context, const ReduceConfigRequest* request, ServerWriter<ReduceConfig>* writer) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceConfig request." << std::endl;
        
        // Respond with saved config.
        ReduceConfig config;
        config.set_output_filepath(last_output_filepath);

        for (auto const& filepath : last_input_filepaths) {
            config.set_input_filepath(filepath);
            writer->Write(config);
        }

        return Status::OK;
    }
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