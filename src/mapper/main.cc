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

using namespace mapreduce;

class MapperListenerServiceImpl final : public MapperListener::Service {
public:
    Status ProcessMapRequest(ServerContext* context, const MapRequest* request, Response* response) override {
        std::cerr << "[MAPPER LISTENER] Received MapRequest." << std::endl;
        std::cerr << " ---------------  id: " << request->id() << std::endl;
        std::cerr << " ---------------  filepath: " << request->filepath() << std::endl;
        std::cerr << " ---------------  execpath: " << request->execpath() << std::endl;
        std::cerr << " ---------------  num_reducers: " << request->num_reducers() << std::endl;
        
        // Save config for later.
        MapConfig config;
        config.set_id(request->id());
        config.set_filepath(request->filepath());
        config.set_num_reducers(request->num_reducers());
        work_queue.add(request->execpath(), config);

        // Run mapper binary
        std::system(std::string(request->execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetMapConfig(ServerContext* context, const MapConfigRequest* request, MapConfig* config) override {
        std::cerr << "[MAPPER LISTENER] Received MapConfig request from " << request->execpath() << std::endl;
        
        // Respond with saved config.
        config->CopyFrom(work_queue.get_one(request->execpath()));

        return Status::OK;
    }

private:
    ListenerWorkQueue<MapConfig> work_queue;
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