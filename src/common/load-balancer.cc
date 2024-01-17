#include "load-balancer.h"

#include <cassert>

namespace mapreduce {

void LoadBalancer::add_worker(std::string const& ip) {
{
    std::lock_guard<std::mutex> lock(mutex);
    round_robin_it = workers.emplace(ip, true).first;
    free_workers++;
}
    cv.notify_one();
}

void LoadBalancer::remove_worker(std::string const& ip) {
    std::lock_guard<std::mutex> lock(mutex);
    workers.erase(ip);
    round_robin_it = workers.begin();
}

void LoadBalancer::free_worker(std::string const& ip) {
{
    std::lock_guard<std::mutex> lock(mutex);
    workers[ip] = true;
    free_workers++;
}
    cv.notify_one();
}

std::string LoadBalancer::get_free_worker() {
    std::unique_lock lock(mutex);
    cv.wait(lock, [this] { return free_workers > 0; });

    while (!round_robin_it->second) {
        if (++round_robin_it == workers.end()) {
            round_robin_it = workers.begin();
        }
    }

    round_robin_it->second = false;
    free_workers--;
    return round_robin_it->first;
}

} // mapreduce