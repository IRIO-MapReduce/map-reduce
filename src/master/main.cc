#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <latch>
#include <thread>
#include <grpc++/grpc++.h>
#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"
#include "../common/cloud_utils.h"
#include "../common/job-manager.h"

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
        
        auto map_group_id = job_manager.register_new_jobs_group(request->num_mappers());

        // Files to be sent to reducers
        std::vector<std::vector<std::string>> intermediate_files(request->num_reducers(), std::vector<std::string>(request->num_mappers()));

        // Send requests to mappers
        for (size_t i = 0; i < request->num_mappers(); i++) {
            std::string part_filepath = get_split_filepath(request->input_filepath(), i);

            JobRequest map_request;
            map_request.set_group_id(map_group_id);
            map_request.set_job_id(i);
            map_request.set_job_type(JobRequest::MAP);
            map_request.set_execpath(request->mapper_execpath());
            map_request.add_input_filepath(part_filepath);
            map_request.set_num_outputs(request->num_reducers());

            job_manager.add_job(map_group_id, map_request);
            
            for (size_t j = 0; j < request->num_reducers(); j++) {
                intermediate_files[j][i] = get_intermediate_filepath(part_filepath, j);
            }
        }

        std::cerr << "[MASTER] Map phase complete, waiting for mappers to finish." << std::endl;
        
        job_manager.wait_for_completion(map_group_id);
        
        std::cerr << "[MASTER] Mappers finished, starting Reduce phase" << std::endl;

        auto reduce_group_id = job_manager.register_new_jobs_group(request->num_reducers());

        for (uint32_t i = 0; i < request->num_reducers(); i++) {
            std::cerr << "[MASTER] Sending ReduceRequest to ReducerListener." << std::endl;

            JobRequest reduce_request;
            reduce_request.set_group_id(reduce_group_id);
            reduce_request.set_job_id(i);
            reduce_request.set_job_type(JobRequest::REDUCE);
            reduce_request.set_execpath(request->reducer_execpath());
            reduce_request.set_output_filepath(request->output_filepath());
            for (auto const& filepath : intermediate_files[i]) {
                reduce_request.add_input_filepath(filepath);
            }

            job_manager.add_job(reduce_group_id, reduce_request);
        }

        std::cerr << "[MASTER] Reduce phase complete, waiting for reducers to finish." << std::endl;

        job_manager.wait_for_completion(reduce_group_id);

        std::cerr << "[MASTER] Finished." << std::endl;

        return Status::OK;
    }

    void start_job_manager() {
        job_manager.start(JOB_MANAGER_ADDRESS);
    }

private:
    JobManager job_manager;
    // Identifier client_id;
    // JobManager job_manager;
    // ClientRequestQueue client_requests;
};

void RunMasterServer() {
    std::cerr << "[MASTER] Started running" << std::endl;
    std::string server_address(MASTER_ADDRESS);
    MasterServiceImpl service;

    std::thread job_manager_thread([&service]() { service.start_job_manager(); });

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[MASTER] Server listening on " << server_address << std::endl;
    server->Wait();

    job_manager_thread.join();
}

int main() {
    RunMasterServer();
    return 0;
}