#include <iostream>
#include <fstream>
#include <sstream>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "mapper.h"
#include "utils.h"
#include "mapreduce.h"
#include "job-manager.h"
#include "cloud_utils.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;

namespace mapreduce {

bool Mapper::get_next_pair(key_t& key, val_t& val) {
    if (!input_file.is_open()) {
        try {
            input_file.open(input_filepath);
        }
        catch (...) {
            std::cerr << "[ERROR] Error opening file" << std::endl;
            return false;
        }
    }

    std::string line;
    if (getline(input_file, line)) {
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, val);
        return true;
    }

    return false;
}

void Mapper::emit(key_t const& key, val_t const& val) {
    size_t h = std::hash<key_t>()(key) % num_reducers;
    std::string filepath = get_intermediate_filepath(input_filepath, h);
    std::ofstream output_file(filepath, std::ios::app);
    output_file << key + "," + val + "\n";
}

void Mapper::start(int argc, char** argv) {
    assert(argc == 1);
    std::cerr << "[MAPPER] Starting worker..." << std::endl;

    std::shared_ptr<Channel> channel = grpc::CreateChannel(get_address(LOCALHOST, WORKER_PORT), grpc::InsecureChannelCredentials());
    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(channel);

    ConfigRequest request;
    request.set_execpath(argv[0]);

    ClientContext context;
    JobRequest job;

    std::cerr << "[MAPPER] Requesting job from listener" << std::endl;
    
    Status status = stub->GetFreeTask(&context, request, &job);

    assert(status.ok());
    assert(job.job_type() == JobRequest::MAP);

    this->group_id = job.group_id();
    this->job_id = job.job_id();
    this->input_filepath = job.input_filepath()[0];
    this->num_reducers = job.num_outputs();
    this->job_manager_address = job.job_manager_address();

    std::cerr << "[MAPPER WORKER] Worker info retrieved, starting map..." << std::endl;

    map();

    std::unique_ptr<JobManagerService::Stub> manager_stub = JobManagerService::NewStub(
        grpc::CreateChannel(this->job_manager_address, grpc::InsecureChannelCredentials())
    );

    ClientContext manager_context;
    JobFinishedRequest finished_request;
    finished_request.set_group_id(this->group_id);
    finished_request.set_job_id(this->job_id);
    Response response;

    std::cerr << "[MAPPER WORKER] Sending MapCompleted to master" << std::endl;

    status = manager_stub->NotifyJobFinished(&manager_context, finished_request, &response);

    assert(status.ok());
    
    std::cerr << "[MAPPER WORKER] Map completed!" << std::endl;
}

} // mapreduce
