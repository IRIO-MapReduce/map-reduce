#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <string>
#include <unordered_map>
#include <mutex>

/**
 * Assumes each worker can perform at most one job at a time.
*/
class LoadBalancer {
public:
    /**
     * Returns the IP of any free worker.
     * Potentially blocking.
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
};

#endif // LOAD_BALANCER_H