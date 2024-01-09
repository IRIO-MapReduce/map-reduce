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

class MasterServiceImpl final : public Master::Service {
public:
    Status ProcessClientRequest(ServerContext* context, const ClientRequest* request, Response* response) override {
        std::cerr << "[MASTER] Received ClientRequest." << std::endl;
        std::cerr << " ---------------  input_filepath: " << request->input_filepath() << std::endl;
        std::cerr << " ---------------  mapper_execpath: " << request->mapper_execpath() << std::endl;
        std::cerr << " ---------------  reducer_execpath: " << request->reducer_execpath() << std::endl;
        std::cerr << " ---------------  num_reducers: " << request->num_reducers() << std::endl;
        std::cerr << " ---------------  split_size_bytes: " << request->split_size_bytes() << std::endl;
        
        size_t n_parts;
        try {
            n_parts = split_file_bytes(request->input_filepath(), request->split_size_bytes());
        } 
        catch (...) {
            std::cerr << "Error splitting file" << std::endl;
            response->set_result("Error splitting file");
            return Status::CANCELLED;
        }

        // Files to be sent to reducers
        // std::vector<std::vector<std::string>> intermediate_files(request->num_reducers(), std::vector<std::string>(n_parts));

        for (size_t i = 0; i < n_parts; i++) {
            std::string part_filepath = get_split_filepath(request->input_filepath(), i);

            MapRequest map_request;
            map_request.set_filepath(part_filepath);
            map_request.set_execpath(request->mapper_execpath());
            map_request.set_num_reducers(request->num_reducers());

            std::string mapper_listener_address(MAPPER_LISTENER_ADDRESS);
            
            std::unique_ptr<MapperListener::Stub> mapper_listener_stub(MapperListener::NewStub(
                grpc::CreateChannel(mapper_listener_address, grpc::InsecureChannelCredentials())
            ));

            Response response;
            grpc::ClientContext context;

            std::cerr << "[MASTER] Sending MapRequest to MapperListener." << std::endl;

            grpc::Status status = mapper_listener_stub->ProcessMapRequest(&context, map_request, &response);
            assert(status.ok());

            std::cerr << "[MASTER] Received response from MapperListener." << std::endl;

            // for (size_t j = 0; j < request->num_reducers(); j++) {
            //     intermediate_files[j][i] = get_intermediate_filepath(part_filepath, j);
            // }
        }

        return Status::OK;
    }
};

void RunMasterServer() {
    std::string server_address(MASTER_ADDRESS);
    MasterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[MASTER] Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunMasterServer();
    return 0;
}