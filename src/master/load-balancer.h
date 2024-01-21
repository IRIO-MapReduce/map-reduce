#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <condition_variable>
#include <string>

#include "health-checker.h"

namespace mapreduce {

class LoadBalancer {
public:
    /**
     * Returns the IP of a worker that should be assigned a new job.
     * Potentially blocks until a worker is available.
     */
    std::string get_worker_ip();

    /**
     * Notifies the load balancer that a worker has finished its job.
     */
    void notify_worker_finished(std::string const& worker_ip);

    /**
     * Starts the load balancer.
     * Blocks, so should be invoked in a separate thread.
     */
    void start();

private:
    /**
     * Return the IP of any worker that is not busy, and marks it as busy.
     * Potentially blocks until a worker is available.
     * Does not check if the worker is healthy.
     */
    std::string get_worker_ip_unchecked();

    /**
     * Synchronizes the state of worker machines with active VM instances on
     * Google Cloud.
     */
    void refresh_workers();

    /**
     * Checks if any of the unhealthy workers is now healthy.
     */
    void check_unhealthy_workers();

    /**
     * Marks worker with a given index as free.
     */
    void release_worker(uint32_t idx);

    /**
     * Tries to acquire a token for a worker.
     * Returns true if successful, false otherwise.
     * If successful, reduces the number of available workers by one and
     * guarantees, that there will be at least one worker available to take a
     * job.
     */
    bool try_acquire_worker();

    /**
     * Tries to acquire a token for a specific worker.
     * If successful, updates this thread as more "busy", thus possibly
     * disallowing other jobs to be executed on this worker.
     * May spuriously fail even if the worker is able to accept more jobs.
     * Returns true if successful.
     */
    bool try_acquire_worker(uint32_t idx);

    HealthChecker health_checker;

    std::shared_mutex mutex;
    std::condition_variable_any cv;
    std::atomic<uint32_t> available_workers;
    std::vector<std::string> worker_ips;
    std::unordered_map<std::string, uint32_t> idx_of_worker;
    std::vector<std::atomic<uint32_t>> busy_threads;
    std::vector<std::string> unhealthy_workers;
};

} // mapreduce

#endif // LOAD_BALANCER_H