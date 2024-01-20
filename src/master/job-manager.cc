#include <cassert>
#include <grpc++/grpc++.h>

#include "job-manager.h"
#include "mapreduce.h"
#include "cloud-utils.h"

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

uint32_t JobManager::register_new_jobs_group(uint32_t num_jobs) {
    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.emplace(next_group_id, std::make_shared<JobGroup>(num_jobs));
    return next_group_id++;
}

void JobManager::add_job(uint32_t group_id, JobRequest const& request) {
    std::cerr << "[JOB MANAGER] Adding job (" << group_id << ", " << request.job_id() << ")" << std::endl;
    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);
        assert(job_groups.contains(group_id));
        job_groups[group_id]->add_job(request);
    }

    auto worker_ip = load_balancer.get_worker_ip();
    auto worker_address = get_address(worker_ip, WORKER_PORT);

    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(
        grpc::CreateChannel(worker_address, grpc::InsecureChannelCredentials())
    );

    Response response;
    grpc::ClientContext context;

    std::cerr << "[JOB MANAGER] Sending JobRequest to Worker (" << worker_ip << ")" << std::endl;
    
    grpc::Status status = stub->ProcessJobRequest(&context, request, &response);

    assert(status.ok());

    std::cerr << "[JOB MANAGER] Request sent successfully" << std::endl;
}

void JobManager::wait_for_completion(uint32_t group_id) {
    /**
     * TODO: Discuss timeout and include it somewhere else in the config.
    */
    static constexpr uint32_t TIMEOUT = 5;

    std::shared_ptr<JobGroup> group;
    
    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);
        assert(job_groups.contains(group_id));
        group = job_groups[group_id];
    }

    std::cerr << "[JOB MANAGER] Waiting for group (" << group_id << ") to complete" << std::endl;
    while (!group->wait_for_completion(TIMEOUT)) {
        auto unfinished_jobs = group->get_unfinished_jobs();
        std::cerr << "[JOB MANAGER] Group (" << group_id << ") timed out. Unfinished jobs: " << std::endl;
        for (const auto& job : unfinished_jobs) {
            std::cerr << "\tJob (" << job.group_id() << ", " << job.job_id() << ")" << std::endl;
            add_job(group_id, job);
        }
    }
    std::cerr << "[JOB MANAGER] Group (" << group_id << ") completed successfully" << std::endl;

    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.erase(group_id);
}

void JobManager::mark_completed(uint32_t group_id, uint32_t job_id) {
    std::cerr << "[JOB MANAGER] Job (" << group_id << ", " << job_id << ") completed" << std::endl;
    std::shared_lock<std::shared_mutex> lock(groups_lock);
    assert(job_groups.contains(group_id));
    job_groups[group_id]->mark_completed(job_id);
}

void JobManager::start(std::string const& address) {
    std::thread load_balancer_thread([this]() { load_balancer.start(); });
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[JOB MANAGER] Server listening on " << address << std::endl;
    server->Wait();
    load_balancer_thread.join();
}

} // mapreduce