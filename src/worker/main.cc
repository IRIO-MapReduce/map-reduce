#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;

using namespace mapreduce;

class WorkerListenerServiceImpl final : public WorkerListener::Service {
    Status ProcessReduceRequest(ServerContext* context, ServerReader<ReduceRequest>* reader, Response* response) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceRequest." << std::endl;
        
        std::vector<ReduceConfig> configs;
        ReduceRequest request;

        while (reader->Read(&request)) {
            ReduceConfig config;
            config.set_id(request.id());
            config.set_input_filepath(request.input_filepath());
            config.set_output_filepath(request.output_filepath());
            configs.push_back(config);
        }

        std::cerr << " ----------------  execpath: " << request.execpath() << std::endl;
        std::cerr << " ----------------  output_filepath: " << request.output_filepath() << std::endl;

        // Save config for later.
        reduce_queue.add(request.execpath(), configs);

        // Run reducer binary
        std::system(std::string(request.execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetReduceConfig(ServerContext* context, const ConfigRequest* request, ServerWriter<ReduceConfig>* writer) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceConfig request." << std::endl;
        
        // Respond with saved config.
        std::vector<ReduceConfig> configs = reduce_queue.get_one(request->execpath());

        for (auto const& config : configs) {
            writer->Write(config);
        }

        return Status::OK;
    }

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
        map_queue.add(request->execpath(), config);

        // Run mapper binary
        std::system(std::string(request->execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetMapConfig(ServerContext* context, const ConfigRequest* request, MapConfig* config) override {
        std::cerr << "[MAPPER LISTENER] Received MapConfig request from " << request->execpath() << std::endl;
        
        // Respond with saved config.
        config->CopyFrom(map_queue.get_one(request->execpath()));

        return Status::OK;
    }

private:
    ListenerWorkQueue<std::vector<ReduceConfig>> reduce_queue;
    ListenerWorkQueue<MapConfig> map_queue;
};

void RunWorkerListenerServer() {
    std::string server_address(MAPPER_LISTENER_ADDRESS);
    WorkerListenerServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[WORKER LISTENER] Server listening on " << server_address << std::endl;

    server->Wait();
}

int main() {
    RunWorkerListenerServer();
    return 0;
}