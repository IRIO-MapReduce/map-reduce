#include "mapreduce.grpc.pb.h"
#include <fstream>
#include <grpc++/grpc++.h>
#include <iostream>
#include <latch>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "../common/cloud-utils.h"
#include "../common/data-structures.h"
#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "job-manager.h"

using grpc::Channel;
using grpc::ClientAsyncReader;
using grpc::ClientAsyncReaderWriter;
using grpc::ClientAsyncResponseReader;
using grpc::ClientAsyncResponseReaderInterface;
using grpc::ClientAsyncWriter;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace mapreduce;

class MasterServiceImpl final : public Master::Service {
public:
    Status ProcessClientRequest(ServerContext* context,
        const ClientRequest* request, ClientResponse* response) override
    {
        log_message("[MASTER] Received ClientRequest.",
            google::logging::type::LogSeverity::INFO,
            {{"input_filepath", request->input_filepath()},
                {"output_filepath", request->output_filepath()},
                {"mapper_execpath", request->mapper_execpath()},
                {"reducer_execpath", request->reducer_execpath()},
                {"num_mappers", std::to_string(request->num_mappers())},
                {"num_reducers", std::to_string(request->num_reducers())}});

        auto map_group_id = job_manager.register_new_jobs_group(
            request->num_mappers(), request->num_reducers());
        auto reduce_group_id = map_group_id + 1;

        for (uint32_t i = 0; i < request->num_mappers(); i++) {
            JobRequest map_request;
            map_request.set_group_id(map_group_id);
            map_request.set_job_id(i);
            map_request.set_job_type(JobRequest::MAP);
            map_request.set_execpath(request->mapper_execpath());
            map_request.set_input_filepath(request->input_filepath());
            map_request.set_num_outputs(request->num_reducers());
            map_request.set_job_manager_address(job_manager_address);

            job_manager.add_job(map_group_id, map_request);
        }

        job_manager.wait_for_completion(map_group_id);

        for (uint32_t i = 0; i < request->num_reducers(); i++) {
            JobRequest reduce_request;
            reduce_request.set_group_id(reduce_group_id);
            reduce_request.set_job_id(i);
            reduce_request.set_job_type(JobRequest::REDUCE);
            reduce_request.set_execpath(request->reducer_execpath());
            reduce_request.set_input_filepath(request->input_filepath());
            reduce_request.set_num_inputs(request->num_mappers());
            reduce_request.set_output_filepath(request->output_filepath());
            reduce_request.set_job_manager_address(job_manager_address);

            job_manager.add_job(reduce_group_id, reduce_request);
        }

        job_manager.wait_for_completion(reduce_group_id);

        response->set_group_id(reduce_group_id);
        return Status::OK;
    }

    void start_job_manager()
    {
        auto master_ip = get_master_ip();
        assert(master_ip.has_value());
        job_manager_address = get_address(master_ip.value(), JOB_MANAGER_PORT);
        job_manager.start(get_address(LISTENING_ADDRESS, JOB_MANAGER_PORT));
    }

private:
    JobManager job_manager;
    std::string job_manager_address;
};

void RunMasterServer()
{
    std::string server_address(get_address(LISTENING_ADDRESS, MASTER_PORT));
    MasterServiceImpl service;

    std::thread job_manager_thread(
        [&service]() { service.start_job_manager(); });

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    log_message("[MASTER] Server listening on " + server_address);
    server->Wait();

    job_manager_thread.join();
}

int main()
{
    RunMasterServer();
    return 0;
}