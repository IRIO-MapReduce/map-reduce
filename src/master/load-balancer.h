#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <string>

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
     * Synchronizes the state of worker machines with active VM instances on Google Cloud.
    */
    void refresh_workers();
    
};

} // mapreduce

#endif // LOAD_BALANCER_H