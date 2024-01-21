#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include <condition_variable>
#include <grpc++/grpc++.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "mapreduce.grpc.pb.h"

#include "../common/data-structures.h"
#include "../common/utils.h"
#include "cloud-utils.h"
#include "load-balancer.h"

using grpc::ServerContext;
using grpc::Status;

namespace mapreduce {

/**
 * Simple load balancer.
 * Takes care of assigning jobs to workers, assuming all workers can have at
 * most one job at a time.
 */
class JobManager final : public JobManagerService::Service {
public:
    /**
     * Returns a new unique request ID. Specifies the pool of jobs that will be
     * assigned as the same synchronized group. If return ID is N, then ids N,
     * N+1 are for requestor disposal. The proposed solution is to use id N for
     * map jobs and id N+1 for reduce jobs.
     */
    uint32_t register_new_jobs_group(uint32_t map_jobs, uint32_t reduce_jobs);

    /**
     * Adds a job to the pool. Used by master to assign jobs to workers.
     * Job request should contain a correct group ID and job ID.
     * That means, 0 < map_request.job_id() < num_jobs holds for group_id and
     * requested jobs should be distinct.
     * Otherwise, the behavior is undefined.
     */
    void add_job(uint32_t group_id, JobRequest const& map_request);

    /**
     * Hangs the thread until all jobs from one group are completed.
     */
    void wait_for_completion(uint32_t group_id);

    /**
     * Starts the server listening on the given address.
     * Blocks, so should be invoked in a separate thread.
     */
    void start(std::string const& address);

    /**
     * Processes a request from a worker, that a job is completed.
     */
    Status NotifyJobFinished(ServerContext* context,
        JobFinishedRequest const* request, Response* response) override
    {
        auto address = uri_to_url(context->peer());
        log_message("[JOB MANAGER] Received JobFinished by: " + address);
        load_balancer.notify_worker_finished(address);
        mark_completed(request->group_id(), request->job_id());
        return Status::OK;
    }

private:
    /**
     * Marks a job as completed. If all jobs from a group are completed,
     * notifies a thread waiting for this group (if any).
     */
    void mark_completed(uint32_t group_id, uint32_t job_id);

    uint32_t next_group_id = 0;
    std::shared_mutex groups_lock;
    std::unordered_map<uint32_t, std::shared_ptr<JobGroup>> job_groups;
    LoadBalancer load_balancer;
    HealthChecker health_checker;
};

} // mapreduce

#endif // JOB_MANAGER_H