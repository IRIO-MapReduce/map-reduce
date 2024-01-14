#ifndef DATA_STRUCTURES_CC
#define DATA_STRUCTURES_CC

#include "data-structures.h"

namespace mapreduce {

template<class V>
void ListenerWorkQueue<V>::add(std::string const& execpath, V const& item) {
    std::lock_guard<std::mutex> lock(mutex);
    work_queue[execpath].push(item);
}

template<class V>
V ListenerWorkQueue<V>::get_one(std::string const& execpath) {
    std::lock_guard<std::mutex> lock(mutex);
    auto& queue = work_queue[execpath];
    assert(!queue.empty());
    V item = queue.front();
    queue.pop();
    return item;
}

void ClientRequestInfo::add_mapper_job(MapRequest const& request) {
    std::lock_guard<std::mutex> lock(mutex);
    ongoing_map_requests[request.id()] = request;
}

void ClientRequestInfo::add_reducer_job(ReduceRequest const& request) {
    std::lock_guard<std::mutex> lock(mutex);
    ongoing_reduce_requests[request.id()] = request;
}

void ClientRequestInfo::complete_mapper_job(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    ongoing_map_requests.erase(id);
    map_latch.count_down();
}

void ClientRequestInfo::complete_reducer_job(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    ongoing_reduce_requests.erase(id);
    reduce_latch.count_down();
}

void ClientRequestInfo::wait_for_map() {
    map_latch.wait();
}

void ClientRequestInfo::wait_for_reduce() {
    reduce_latch.wait();
}

ClientRequestInfo& ClientRequestQueue::add_request(uint32_t id, ClientRequest const& request) {
    std::lock_guard<std::mutex> lock(mutex);
    assert(ongoing_requests.find(id) == ongoing_requests.end());
    std::cerr << "Adding request with id: " << id << std::endl;
    return ongoing_requests.emplace(id, request).first->second;
}

void ClientRequestQueue::complete_request(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    assert(ongoing_requests.find(id) != ongoing_requests.end());
    ongoing_requests.erase(id);
}

ClientRequestInfo& ClientRequestQueue::get_request_info(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = ongoing_requests.find(id);
    assert(it != ongoing_requests.end());
    return it->second;
}

} // mapreduce

#endif // DATA_STRUCTURES_CC