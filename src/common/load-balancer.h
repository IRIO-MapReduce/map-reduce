#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

namespace mapreduce {

/**
 * Simple load balancer.
 * Takes care of assigning jobs to workers, assuming all workers can have at most one job at a time.
*/
class LoadBalancer {
public:
    /**
     * Returns the IP of any free worker.
     * Potentially blocking, while waiting for a free worker.
    */
    std::string get_free_worker();

    /**
     * Mark worker as free, can be assigned to another job.
    */
    void free_worker(std::string const& ip);

    /**
     * Add worker to the pool.
    */
    void add_worker(std::string const& ip);

    /**
     * Remove worker from the pool.
    */
    void remove_worker(std::string const& ip);

private:
    std::unordered_map<std::string, bool> workers;
    std::unordered_map<std::string, bool>::iterator round_robin_it;
    std::mutex mutex;
    std::condition_variable cv;
    size_t free_workers = 0;
};

} // mapreduce

#endif // LOAD_BALANCER_H