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

using namespace mapreduce;

static std::string last_filepath;
static uint32_t last_num_reducers;

class MapperListenerServiceImpl final : public MapperListener::Service {
public:
    Status ProcessMapRequest(ServerContext* context, const MapRequest* request, Response* response) override {
        std::cerr << "[MAPPER LISTENER] Received MapRequest." << std::endl;
        std::cerr << " ---------------  filepath: " << request->filepath() << std::endl;
        std::cerr << " ---------------  execpath: " << request->execpath() << std::endl;
        std::cerr << " ---------------  num_reducers: " << request->num_reducers() << std::endl;
        
        // Save config for later.
        last_filepath = request->filepath();
        last_num_reducers = request->num_reducers();

        // Run mapper binary
        std::system(request->execpath().c_str());

        return Status::OK;
    }

    Status GetMapConfig(ServerContext* context, const MapConfigRequest* request, MapConfig* config) override {
        std::cerr << "[MAPPER LISTENER] Received MapConfig request." << std::endl;
        
        // Respond with saved config.
        config->set_filepath(last_filepath);
        config->set_num_reducers(last_num_reducers);

        return Status::OK;
    }
};

void RunMapperListenerServer() {
    std::string server_address(MAPPER_LISTENER_ADDRESS);
    MapperListenerServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cerr << "[MAPPER LISTENER] Server listening on " << server_address << std::endl;
    
    server->Wait();
}

int main() {
    RunMapperListenerServer();
    return 0;
}