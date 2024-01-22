#include <cassert>
#include <grpc++/grpc++.h>

#include "cloud-utils.h"
#include "job-manager.h"
#include "mapreduce.h"

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

namespace mapreduce {

uint32_t JobManager::register_new_jobs_group(
    uint32_t map_jobs, uint32_t reduce_jobs)
{
    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.emplace(next_group_id, std::make_shared<JobGroup>(map_jobs));
    job_groups.emplace(
        next_group_id + 1, std::make_shared<JobGroup>(reduce_jobs));
    uint32_t ret = next_group_id;
    next_group_id += 2;
    return ret;
}

void JobManager::add_job(uint32_t group_id, JobRequest const& request)
{
    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);

        /**
         * Could be already completed if timed out.
         */
        auto it = job_groups.find(group_id);
        if (it != job_groups.end()) {
            it->second->add_job(request);
        }
    }

    auto worker_ip = load_balancer.get_worker_ip();
    auto worker_address = get_address(worker_ip, WORKER_PORT);

    std::unique_ptr<Worker::Stub> stub = Worker::NewStub(grpc::CreateChannel(
        worker_address, grpc::InsecureChannelCredentials()));

    Response response;
    grpc::ClientContext context;
    grpc::Status status = stub->ProcessJobRequest(&context, request, &response);

    if (!status.ok()) {
        log_message("[JOB MANAGER] Worker (" + worker_ip
                + ") failed to process JobRequest",
            google::logging::type::LogSeverity::ERROR);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        add_job(group_id, request);
    }
}

void JobManager::wait_for_completion(uint32_t group_id)
{
    /**
     * TODO: Discuss timeout and include it somewhere else in the config.
     */
    static constexpr uint32_t TIMEOUT = 30;

    std::shared_ptr<JobGroup> group;

    {
        std::shared_lock<std::shared_mutex> lock(groups_lock);
        assert(job_groups.contains(group_id));
        group = job_groups[group_id];
    }

    while (!group->wait_for_completion(TIMEOUT)) {
        auto unfinished_jobs = group->get_unfinished_jobs();

        log_message("[JOB MANAGER] Group (" + std::to_string(group_id)
                + ") timed out, resubmitting jobs.",
            google::logging::type::LogSeverity::WARNING);

            for (const auto& job : unfinished_jobs)
                add_job(group_id, job);
    }

    std::unique_lock<std::shared_mutex> lock(groups_lock);
    job_groups.erase(group_id);
}

void JobManager::mark_completed(uint32_t group_id, uint32_t job_id)
{
    std::shared_lock<std::shared_mutex> lock(groups_lock);

    /**
     * Could be already completed if timed out.
     */
    auto it = job_groups.find(group_id);
    if (it != job_groups.end()) {
        it->second->mark_completed(job_id);
    }
}

void JobManager::start(std::string const& address)
{
    std::thread load_balancer_thread([this]() { load_balancer.start(); });
    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    log_message("[JOB MANAGER] Server listening on " + address);
    server->Wait();
    load_balancer_thread.join();
}

} // mapreduce