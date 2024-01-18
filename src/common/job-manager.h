#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "utils.h"
#include "data-structures.h"

using grpc::Status;
using grpc::ServerContext;

namespace mapreduce {

/**
 * Simple load balancer.
 * Takes care of assigning jobs to workers, assuming all workers can have at most one job at a time.
*/
class JobManager final : public JobManagerService::Service {
public:
    /**
     * Returns the IP of any free worker.
     * Potentially blocking, while waiting for a free worker.
    */
    // std::string get_free_worker();

    /**
     * Mark worker as free, can be assigned to another job.
    */
    // void free_worker(std::string const& ip);

    /**
     * Add worker to the pool.
    */
    // void add_worker(std::string const& ip);

    /**
     * Remove worker from the pool.
    */
    // void remove_worker(std::string const& ip);

    /**
     * Returns a new unique request ID. Specifies the pool of jobs that will be assigned
     * as the same synchronized group.
    */
    uint32_t register_new_jobs_group(uint32_t num_jobs);

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
     * Marks a job as completed. If all jobs from a group are completed,
     * notifies a thread waiting for this group (if any).
    */
    void mark_completed(uint32_t group_id, uint32_t job_id);

    /**
     * Starts the server listening on the given address.
     * Blocks, so should be invoked in a separate thread.
    */
    void start(std::string const& address);

    /**
     * TODO: cannot override in .cc file, maybe some elegant solution is possible.
    */
    Status NotifyJobFinished(ServerContext* context, JobFinishedRequest const* request, Response* response) override {
        std::cerr << "[JOB_MANAGER] Received JobFinishedRequest from " << context->peer() << std::endl;
        mark_completed(request->group_id(), request->job_id());
        return Status::OK;
    }

private:
//     std::unordered_map<std::string, bool> workers;
//     std::unordered_map<std::string, bool>::iterator round_robin_it;
//     std::mutex mutex;
//     std::condition_variable cv;
//     size_t free_workers = 0;
    uint32_t next_group_id;
    std::shared_mutex groups_lock;
    std::unordered_map<uint32_t, std::shared_ptr<JobGroup>> job_groups;
};

} // mapreduce

#endif // JOB_MANAGER_H