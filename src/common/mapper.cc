#include <iostream>
#include <fstream>
#include <sstream>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "mapper.h"
#include "utils.h"
#include "mapreduce.h"
#include "cloud-utils.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;

namespace mapreduce {

bool Mapper::get_next_pair(key_t& key, val_t& val) {
    if (!input_file.is_open()) {
        std::string filepath = input_filepath;
        filepath = combine_filepath(filepath, job_id);

        try {
            input_file.open(filepath);
        }
        catch (...) {
            std::cerr << "[ERROR] Error opening file " << filepath << std::endl;
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

    std::string filepath = input_filepath;
    filepath = combine_filepath(filepath, job_id);
    filepath = combine_filepath(filepath, group_id);
    filepath = combine_filepath(filepath, h);
    filepath = combine_filepath(filepath, hash);

    std::cerr << "[MAPPER] Emitting (" << key << ", " << val << ") to " << filepath << std::endl;
    std::ofstream output_file(filepath, std::ios::app);
    output_file << key + "," + val + "\n";
}

void Mapper::start(int argc, char** argv) {
    assert(argc == 1);
    std::cerr << "[MAPPER] Starting worker..." << std::endl;

    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    hash = get_random_string();

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
    this->input_filepath = job.input_filepath();
    this->num_reducers = job.num_outputs();
    this->job_manager_address = job.job_manager_address();

    std::cerr << "[MAPPER] Job received: (" << this->group_id << ", " << this->job_id << ")" << std::endl;
    std::cerr << "[MAPPER] Input filepath: " << this->input_filepath << std::endl;
    std::cerr << "[MAPPER] Num reducers: " << this->num_reducers << std::endl;
    std::cerr << "[MAPPER] Job manager address: " << this->job_manager_address << std::endl;
    std::cerr << "[MAPPER] Worker info retrieved, starting map..." << std::endl;

    map();

    for (uint32_t i = 0; i < num_reducers; i++) {
        std::string hashed_filepath = input_filepath;
        hashed_filepath = combine_filepath(hashed_filepath, job_id);
        hashed_filepath = combine_filepath(hashed_filepath, group_id);
        hashed_filepath = combine_filepath(hashed_filepath, i);
        hashed_filepath = combine_filepath(hashed_filepath, hash);

        std::string final_filepath = unhash_filepath(hashed_filepath);

        try {
            /**
             * TODO: verify rename, maybe try use std::filesystem::rename or C-style rename.
            */
            std::cerr << "[MAPPER] Renaming " << hashed_filepath << " to " << final_filepath << std::endl;
            std::rename(hashed_filepath.c_str(), final_filepath.c_str());
        }
        catch (...) {
            /**
             * TODO: handle error (should exit or pass the request?)
            */
            std::cerr << "[ERROR] Error renaming file" << hashed_filepath << " to " << final_filepath << std::endl;
        }
    }

    std::unique_ptr<JobManagerService::Stub> manager_stub = JobManagerService::NewStub(
        grpc::CreateChannel(this->job_manager_address, grpc::InsecureChannelCredentials())
    );

    ClientContext manager_context;
    JobFinishedRequest finished_request;
    finished_request.set_group_id(this->group_id);
    finished_request.set_job_id(this->job_id);
    Response response;

    std::cerr << "[MAPPER] Sending MapCompleted to master" << std::endl;

    status = manager_stub->NotifyJobFinished(&manager_context, finished_request, &response);

    assert(status.ok());
    
    std::cerr << "[MAPPER] Map completed!" << std::endl;
}

} // mapreduce
