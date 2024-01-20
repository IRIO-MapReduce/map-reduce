#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <condition_variable>

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
        : num_jobs(num_jobs_),
          jobs(num_jobs), completed(num_jobs) 
        {
            /**
             * TODO: Should definitely never happen if I underestand c++ reference correctly.
            */
            for (uint32_t i = 0; i < num_jobs; ++i) {
                assert(completed[i] == false);
            }
        }

    /**
     * Register a job request. Should be called num_jobs times, and all jobs ids should
     * be distinct and in range [0, num_jobs).
    */
    void add_job(JobRequest const& request);

    /**
     * Hangs the thread until all jobs are completed.
     * Returns true if all jobs are completed, false if timeout is reached.
    */
    bool wait_for_completion(uint32_t timeout);

    /**
     * Marks a job as completed. If all jobs are completed, notifies a 
     * thread waiting for this group (if any).
    */
    void mark_completed(uint32_t id);

    /**
     * Returns all jobs that are not completed yet.
     * Should be called after wait_for_completion, to potentially repeat unfinished jobs.
     * There is a possibility that result contains jobs that finished in the meantime, 
     * but all jobs that are not completed are guaranteed to be in the result.
    */
    std::vector<JobRequest> get_unfinished_jobs();

private:
    std::atomic<uint32_t> num_jobs;
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<JobRequest> jobs;
    std::vector<std::atomic<bool>> completed;
};

} // mapreduce

#endif // DATA_STRUCTURES_H