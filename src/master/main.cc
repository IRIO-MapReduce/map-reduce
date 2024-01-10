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
        std::cerr << " ---------------  output_filepath: " << request->output_filepath() << std::endl;
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
        std::vector<std::vector<std::string>> intermediate_files(request->num_reducers(), std::vector<std::string>(n_parts));

        // Send requests to mappers
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

            for (size_t j = 0; j < request->num_reducers(); j++) {
                intermediate_files[j][i] = get_intermediate_filepath(part_filepath, j);
            }
        }

        // Wait for mappers to end lol
        // [TODO] Change it for async waiting on mappers responses, and
        //  ----  send ReduceRequests as soon as all mappers finish.
        sleep(3);

        for (uint32_t i = 0; i < request->num_reducers(); i++) {
            std::cerr << "[MASTER] Sending ReduceRequest to ReducerListener." << std::endl;

            std::string reducer_listener_address(REDUCER_LISTENER_ADDRESS);

            std::unique_ptr<ReducerListener::Stub> reducer_listener_stub(ReducerListener::NewStub(
                grpc::CreateChannel(reducer_listener_address, grpc::InsecureChannelCredentials())
            ));

            Response reducer_response;

            grpc::ClientContext context;
            std::shared_ptr<grpc::ClientWriter<ReduceRequest>> writer(
                reducer_listener_stub->ProcessReduceRequest(&context, &reducer_response));

            ReduceRequest reduce_request;
            reduce_request.set_execpath(request->reducer_execpath());
            reduce_request.set_output_filepath(request->output_filepath()); // [TODO] Change!!!!

            for (auto const& filepath : intermediate_files[i]) {
                reduce_request.set_input_filepath(filepath);

                std::cerr << "[MASTER] Passing filepath: " << filepath << std::endl;

                assert (writer->Write(reduce_request));
            }

            std::cerr << "[MASTER] Done sending ReduceRequests to that one listener" << std::endl;

            writer->WritesDone();
            grpc::Status status = writer->Finish();

            assert(status.ok());
        }

        std::cerr << "[MASTER] Job done" << std::endl;

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