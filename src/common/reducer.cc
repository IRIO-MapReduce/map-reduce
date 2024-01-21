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

void Reducer::open_file()
{
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    hash = get_random_string();
    output_filepath = combine_filepath(output_filepath, job_id);
    output_filepath = combine_filepath(output_filepath, group_id);
    output_filepath = combine_filepath(output_filepath, hash);

    try {
        output_file.open(output_filepath);
    } catch (std::exception& e) {
        log_message("[REDUCER] Error opening file " + output_filepath
                + ", error code: " + e.what(),
            google::logging::type::LogSeverity::ERROR);

        throw e;
    }
}

void Reducer::close_file()
{
    try {
        output_file.close();
    } catch (std::exception& e) {
        log_message("[REDUCER] Error closing file " + output_filepath,
            google::logging::type::LogSeverity::ERROR);

        throw e;
    }

    std::string final_filepath = unhash_filepath(output_filepath);
    if (std::filesystem::exists(output_filepath)) {
        try {
            std::filesystem::rename(
                output_filepath.c_str(), final_filepath.c_str());
        } catch (std::exception& e) {
            log_message("[REDUCER] Error renaming file " + output_filepath
                    + " to " + final_filepath + ", error code: " + e.what(),
                google::logging::type::LogSeverity::ERROR);

            throw e;
        }
    }
}

bool Reducer::get_next_pair(key_t& key, val_t& val)
{
    if (!input_file.is_open()) {
        if (current_mapper == num_mappers)
            return false;

        std::string filepath = input_filepath;
        filepath = combine_filepath(filepath, current_mapper);
        filepath = combine_filepath(filepath, group_id - 1);
        filepath = combine_filepath(filepath, job_id);

        try {
            input_file.open(filepath);
        } catch (std::exception& e) {
            log_message("[REDUCER] Error opening file " + filepath,
                google::logging::type::LogSeverity::ERROR);

            throw e;
        }
    }

    /**
     * TODO: Optimize (use low level functions, do all manually, read in
     * batches)
     */
    std::string line;
    if (getline(input_file, line)) {
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, val);
        return true;
    }

    current_mapper++;

    try {
        input_file.close();
    } catch (std::exception& e) {
        log_message("[REDUCER] Error closing file " + input_filepath,
            google::logging::type::LogSeverity::ERROR);

        throw e;
    }

    return get_next_pair(key, val);
}

void Reducer::emit(key_t const& key, val_t const& val)
{
    output_file << key + "," + val + "\n";
}

void Reducer::start(int argc, char** argv)
{
    assert(argc == 1);

    std::shared_ptr<Channel> channel
        = grpc::CreateChannel(get_address(LOCALHOST, WORKER_PORT),
            grpc::InsecureChannelCredentials());
    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(channel);
    ConfigRequest request;
    request.set_execpath(argv[0]);
    ClientContext context;
    JobRequest job;

    Status status = stub->GetFreeTask(&context, request, &job);
    assert(status.ok());
    assert(job.job_type() == JobRequest::REDUCE);

    this->group_id = job.group_id();
    this->job_id = job.job_id();
    this->input_filepath = job.input_filepath();
    this->output_filepath = job.output_filepath();
    this->job_manager_address = job.job_manager_address();
    this->num_mappers = job.num_inputs();

    open_file();
    reduce();
    close_file();

    std::unique_ptr<JobManagerService::Stub> manager_stub
        = JobManagerService::NewStub(grpc::CreateChannel(
            this->job_manager_address, grpc::InsecureChannelCredentials()));

    ClientContext manager_context;
    JobFinishedRequest finished_request;
    finished_request.set_group_id(this->group_id);
    finished_request.set_job_id(this->job_id);
    Response response;

    status = manager_stub->NotifyJobFinished(
        &manager_context, finished_request, &response);

    if (!status.ok()) {
        log_message(
            "[REDUCER] Error notifying job manager about finished reduce()",
            google::logging::type::LogSeverity::ERROR,
            {{"group_id", std::to_string(this->group_id)},
                {"job_id", std::to_string(this->job_id)}});
    }
}

} // mapreduce