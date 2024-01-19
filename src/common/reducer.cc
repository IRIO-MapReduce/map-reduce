#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "reducer.h"
#include "utils.h"
#include "mapreduce.h"
#include "cloud-utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

bool Reducer::get_next_pair(key_t& key, val_t& val) {
    if (!input_file.is_open()) {
        if (input_filepaths.empty()) {
            return false;
        }

        try {
            input_file.open(input_filepaths.back());
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

    input_file.close();
    input_filepaths.pop_back();

    return get_next_pair(key, val);
}

void Reducer::emit(key_t const& key, val_t const& val) {
    std::ofstream output_file(output_filepath, std::ios::app);
    output_file << key + "," + val + "\n";  
}

void Reducer::start(int argc, char** argv) {
    assert(argc == 1);
    std::cerr << "[REDUCER] Starting worker..." << std::endl;

    std::shared_ptr<Channel> channel = grpc::CreateChannel(get_address(LOCALHOST, WORKER_PORT), grpc::InsecureChannelCredentials());
    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(channel);

    ConfigRequest request;
    request.set_execpath(argv[0]);

    ClientContext context;
    JobRequest job;

    std::cerr << "[REDUCER] Requesting job from listener" << std::endl;

    Status status = stub->GetFreeTask(&context, request, &job);

    assert(status.ok());
    assert(job.job_type() == JobRequest::REDUCE);

    this->group_id = job.group_id();
    this->job_id = job.job_id();
    for (auto const& filepath : job.input_filepath()) {
        this->input_filepaths.push_back(filepath);
    }
    this->output_filepath = job.output_filepath();
    this->job_manager_address = job.job_manager_address();
    
    std::cerr << "[REDUCER] Starting reduce()" << std::endl;

    reduce();

    std::unique_ptr<JobManagerService::Stub> manager_stub = JobManagerService::NewStub(
        grpc::CreateChannel(this->job_manager_address, grpc::InsecureChannelCredentials())
    );

    ClientContext manager_context;
    JobFinishedRequest finished_request;
    finished_request.set_group_id(this->group_id);
    finished_request.set_job_id(this->job_id);
    Response response;

    std::cerr << "[REDUCER] Sending ReduceCompleted to master" << std::endl;

    status = manager_stub->NotifyJobFinished(&manager_context, finished_request, &response);

    assert(status.ok());
    
    std::cerr << "[REDUCER] Reduce completed!" << std::endl;
}
    

} // mapreduce