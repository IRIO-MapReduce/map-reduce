#include <thread>
#include <iostream>

#include "load-balancer.h"

namespace mapreduce {

/**
 * TODO: Implement
*/
std::string LoadBalancer::get_worker_ip() {
    return "0.0.0.0";
}

/**
 * TODO: implement
*/
void LoadBalancer::notify_worker_finished(std::string const& worker_ip) {
    return;
}

void LoadBalancer::start() {
    while (true) {
        std::cerr << "[LOAD BALANCER] Refreshing workers..." << std::endl;
        refresh_workers();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

/**
 * TODO: implement
*/
void LoadBalancer::refresh_workers() {
    return;
}

} // mapreduce