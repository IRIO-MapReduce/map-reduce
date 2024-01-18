#include <cassert>
#include <grpc++/grpc++.h>

#include "job-manager.h"
#include "mapreduce.h"

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

namespace mapreduce {

// void JobManager::add_worker(std::string const& ip) {
// {
//     std::lock_guard<std::mutex> lock(mutex);
//     round_robin_it = workers.emplace(ip, true).first;
//     free_workers++;
// }
//     cv.notify_one();
// }

// void JobManager::remove_worker(std::string const& ip) {
//     std::lock_guard<std::mutex> lock(mutex);
//     workers.erase(ip);
//     round_robin_it = workers.begin();
// }

// void JobManager::free_worker(std::string const& ip) {
// {
//     std::lock_guard<std::mutex> lock(mutex);
//     workers[ip] = true;
//     free_workers++;
// }
//     cv.notify_one();
// }

// std::string JobManager::get_free_worker() {
//     std::unique_lock lock(mutex);
//     cv.wait(lock, [this] { return free_workers > 0; });

//     while (!round_robin_it->second) {
//         if (++round_robin_it == workers.end()) {
//             round_robin_it = workers.begin();
//         }
//     }

//     round_robin_it->second = false;
//     free_workers--;
//     return round_robin_it->first;
// }

uint32_t JobManager::register_new_jobs_group(uint32_t num_jobs) {
    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.emplace(next_group_id, std::make_shared<JobGroup>(num_jobs));
    return next_group_id++;
}

void JobManager::add_job(uint32_t group_id, JobRequest const& request) {
    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);
        assert(job_groups.contains(group_id));
        job_groups[group_id]->add_job(request);
    }

    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(
        grpc::CreateChannel(WORKER_ADDRESS, grpc::InsecureChannelCredentials())
    );

    Response response;
    grpc::ClientContext context;

    std::cerr << "[JOB_MANAGER] Sending JobRequest to Worker." << std::endl;
    
    grpc::Status status = stub->ProcessJobRequest(&context, request, &response);

    assert(status.ok());

    std::cerr << "[JOB_MANAGER] Done sending JobRequest to Worker." << std::endl;
}

void JobManager::wait_for_completion(uint32_t group_id) {
    std::shared_ptr<JobGroup> group;
    
    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);
        assert(job_groups.contains(group_id));
        group = job_groups[group_id];
    }

    group->wait_for_completion();

    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.erase(group_id);
}

void JobManager::mark_completed(uint32_t group_id, uint32_t job_id) {
    std::shared_lock<std::shared_mutex> lock(groups_lock);
    assert(job_groups.contains(group_id));
    job_groups[group_id]->mark_completed(job_id);
}

void JobManager::start(std::string const& address) {
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[JOB_MANAGER] Server listening on " << address << std::endl;
    server->Wait();
}

} // mapreduce