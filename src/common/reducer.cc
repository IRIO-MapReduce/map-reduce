#include <cassert>
#include <filesystem>
#include <fstream>
#include <grpc++/grpc++.h>
#include <iostream>
#include <sstream>

#include "mapreduce.grpc.pb.h"

#include "cloud-utils.h"
#include "mapreduce.h"
#include "reducer.h"
#include "utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

bool Reducer::get_next_pair(key_t& key, val_t& val)
{
    if (!input_file.is_open()) {
        if (current_mapper == num_mappers) {
            return false;
        }

        std::string filepath = input_filepath;
        filepath = combine_filepath(filepath, current_mapper);
        filepath = combine_filepath(filepath, group_id - 1);
        filepath = combine_filepath(filepath, job_id);

        // log_message("[REDUCER] Opening file " + filepath);

        try {
            input_file.open(filepath);
        } catch (...) {
            log_message("[REDUCER] Error opening file " + filepath,
                google::logging::type::LogSeverity::ERROR);
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
    current_mapper++;

    return get_next_pair(key, val);
}

void Reducer::emit(key_t const& key, val_t const& val)
{
    std::string filepath = output_filepath;
    filepath = combine_filepath(filepath, job_id);
    filepath = combine_filepath(filepath, group_id);
    filepath = combine_filepath(filepath, hash);

    std::ofstream output_file(filepath, std::ios::app);
    // std::cerr << "[REDUCER] Emitting (" << key << ", " << val << ") to "
    //           << filepath << std::endl;
    output_file << key + "," + val + "\n";
}

void Reducer::start(int argc, char** argv)
{
    assert(argc == 1);
    // log_message("[REDUCER] Starting worker...");

    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    hash = get_random_string();

    std::shared_ptr<Channel> channel
        = grpc::CreateChannel(get_address(LOCALHOST, WORKER_PORT),
            grpc::InsecureChannelCredentials());
    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(channel);

    ConfigRequest request;
    request.set_execpath(argv[0]);

    ClientContext context;
    JobRequest job;

    // log_message("[REDUCER] Requesting job from listener");

    Status status = stub->GetFreeTask(&context, request, &job);

    assert(status.ok());
    assert(job.job_type() == JobRequest::REDUCE);

    this->group_id = job.group_id();
    this->job_id = job.job_id();
    this->input_filepath = job.input_filepath();
    this->output_filepath = job.output_filepath();
    this->job_manager_address = job.job_manager_address();
    this->num_mappers = job.num_inputs();

    // log_message("[REDUCER] Starting reduce()",
    //     google::logging::type::LogSeverity::INFO);
    reduce();

    std::string hashed_output_filepath = output_filepath;
    hashed_output_filepath = combine_filepath(hashed_output_filepath, job_id);
    hashed_output_filepath = combine_filepath(hashed_output_filepath, group_id);
    hashed_output_filepath = combine_filepath(hashed_output_filepath, hash);

    std::string final_output_filepath = unhash_filepath(hashed_output_filepath);

    /**
     * TODO: verify rename, maybe try use std::filesystem::rename or C-style
     * rename.
     */
    if (std::filesystem::exists(hashed_output_filepath)) {
        // log_message("[REDUCER] Renaming " + hashed_output_filepath + " to "
        //     + final_output_filepath);

        try {
            std::filesystem::rename(
                hashed_output_filepath.c_str(), final_output_filepath.c_str());
        } catch (std::exception& e) {
            log_message("[REDUCER] Error renaming file "
                    + hashed_output_filepath + " to " + final_output_filepath
                    + ", error code: " + e.what(),
                google::logging::type::LogSeverity::ERROR);
        }
    }

    std::unique_ptr<JobManagerService::Stub> manager_stub
        = JobManagerService::NewStub(grpc::CreateChannel(
            this->job_manager_address, grpc::InsecureChannelCredentials()));

    ClientContext manager_context;
    JobFinishedRequest finished_request;
    finished_request.set_group_id(this->group_id);
    finished_request.set_job_id(this->job_id);
    Response response;

    // log_message("[REDUCER] Sending ReduceCompleted to master");

    status = manager_stub->NotifyJobFinished(
        &manager_context, finished_request, &response);

    assert(status.ok());

    // log_message("[REDUCER] Reduce completed!",
    //     google::logging::type::LogSeverity::INFO);
}

} // mapreduce