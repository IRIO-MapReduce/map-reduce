#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <latch>

#include "mapreduce.grpc.pb.h"

#include "utils.h"

namespace mapreduce {

/**
 * A queue of work items, held by a worker server to serve requests to client binaries.
*/
class WorkQueue {
public:
    /**
     * Adds a new work item to the queue. Items are grouped by execpath.
    */
    void add(JobRequest const& request);

    /**
     * Returns any work item that can be processed by worker with a given execpath. 
     * Assumes that there is at least one item in the queue.
    */
    JobRequest get_one(std::string const& execpath);

private:
    std::unordered_map<std::string, std::queue<JobRequest>> work_queue;
    std::mutex mutex;
};

/**
 * Contains all information about specific group of jobs.
*/
class JobGroup {
public:
    JobGroup(uint32_t num_jobs_)
        : num_jobs(num_jobs_), latch(num_jobs), 
          jobs(num_jobs), completed(num_jobs) 
        {
            for (uint32_t i = 0; i < num_jobs; ++i) {
                completed[i] = false;
            }
        }

    /**
     * Register a job request. Should be called num_jobs times, and all jobs ids should
     * be distinct and in range [0, num_jobs).
    */
    void add_job(JobRequest const& request);

    /**
     * Hangs the thread until all jobs are completed.
    */
    void wait_for_completion();

    /**
     * Marks a job as completed. If all jobs are completed, notifies a 
     * thread waiting for this group (if any).
    */
    void mark_completed(uint32_t id);

private:
    uint32_t num_jobs;
    std::latch latch;
    std::vector<JobRequest> jobs;

    // Atomic, since data race can occur when task is repeated, and is finished by two workers
    // at the same time.
    std::vector<std::atomic<bool>> completed;
};

/**
 * A structure that hold all informations needed by the master to forward requests to mappers / reducers
 * and keep track of their status.
*/
// class ClientRequestQueue {
// public:
//     ClientRequestInfo& add_request(uint32_t req_id, ClientRequest const& request);

//     void complete_request(uint32_t req_id);

//     ClientRequestInfo& get_request_info(uint32_t req_id);

// private:
//     std::unordered_map<uint32_t, ClientRequestInfo> ongoing_requests;
//     std::mutex mutex;
// };

} // mapreduce

#endif // DATA_STRUCTURES_H