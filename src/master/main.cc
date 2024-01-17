#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <grpc++/grpc++.h>
#include <latch>
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

class MasterServiceImpl final : public Master::Service {
public:
    Status ProcessClientRequest(ServerContext* context, const ClientRequest* request, Response* response) override {
        std::cerr << "[MASTER] Received ClientRequest." << std::endl;
        std::cerr << " ---------------  input_filepath: " << request->input_filepath() << std::endl;
        std::cerr << " ---------------  output_filepath: " << request->output_filepath() << std::endl;
        std::cerr << " ---------------  mapper_execpath: " << request->mapper_execpath() << std::endl;
        std::cerr << " ---------------  reducer_execpath: " << request->reducer_execpath() << std::endl;
        std::cerr << " ---------------  num_mappers: " << request->num_mappers() << std::endl;
        std::cerr << " ---------------  num_reducers: " << request->num_reducers() << std::endl;
        
        auto client_req_id = client_id.get_next();
        auto& request_info = client_requests.add_request(client_req_id, *request);

        // Files to be sent to reducers
        std::vector<std::vector<std::string>> intermediate_files(request->num_reducers(), std::vector<std::string>(request->num_mappers()));

        // Send requests to mappers
        for (size_t i = 0; i < request->num_mappers(); i++) {
            std::string part_filepath = get_split_filepath(request->input_filepath(), i);

            MapRequest map_request;
            map_request.set_id(make_req_id(client_req_id, i));
            map_request.set_filepath(part_filepath);
            map_request.set_execpath(request->mapper_execpath());
            map_request.set_num_reducers(request->num_reducers());
            
            request_info.add_mapper_job(map_request);

            std::string mapper_listener_address(MAPPER_LISTENER_ADDRESS);
            std::cerr << " >>> " << mapper_listener_address << " <<< " << std::endl;


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

        std::cerr << "[MASTER] Map phase complete, waiting for mappers to finish." << std::endl;
        
        request_info.wait_for_map();
        
        std::cerr << "[MASTER] Mappers finished, starting Reduce phase" << std::endl;

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
            reduce_request.set_id(make_req_id(client_req_id, i));
            reduce_request.set_execpath(request->reducer_execpath());
            reduce_request.set_output_filepath(request->output_filepath());

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

        std::cerr << "[MASTER] Reduce phase complete, waiting for reducers to finish." << std::endl;

        request_info.wait_for_reduce();

        std::cerr << "[MASTER] Job done" << std::endl;

        return Status::OK;
    }

    Status NotifyMapFinished(ServerContext* context, const JobId* job_id, Response* response) override {
        std::cerr << "[MASTER] Received MapFinished Notification." << std::endl;
        std::cerr << " ---------------  id: " << job_id->id() << std::endl;

        auto [client_id, map_id] = extract_ids(job_id->id());
        auto &request_info = client_requests.get_request_info(client_id);
        request_info.complete_mapper_job(map_id);

        return Status::OK;
    }

    Status NotifyReduceFinished(ServerContext* context, const JobId* job_id, Response* response) override {
        std::cerr << "[MASTER] Received ReduceFinished Notification." << std::endl;
        std::cerr << " ---------------  id: " << job_id->id() << std::endl;

        auto [client_id, reduce_id] = extract_ids(job_id->id());
        auto &request_info = client_requests.get_request_info(client_id);
        request_info.complete_reducer_job(reduce_id);

        return Status::OK;
    }

private:
    Identifier client_id;

    ClientRequestQueue client_requests;
};

void RunMasterServer() {
    std::cout << "[MASTER] Started running" << std::endl;
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