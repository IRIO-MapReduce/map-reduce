#include "data-structures.h"

namespace mapreduce {

void WorkQueue::add(JobRequest const& item) {
    std::lock_guard<std::mutex> lock(mutex);
    work_queue[item.execpath()].push(item);
}

JobRequest WorkQueue::get_one(std::string const& execpath) {
    std::lock_guard<std::mutex> lock(mutex);
    auto& queue = work_queue[execpath];
    assert(!queue.empty());
    JobRequest item = queue.front();
    queue.pop();
    return item;
}

void JobGroup::add_job(JobRequest const& request) {
    uint32_t id = request.job_id();
    jobs[id] = request;
}

void JobGroup::wait_for_completion() {
    latch.wait();
}

void JobGroup::mark_completed(uint32_t id) {
    bool already_completed = completed[id].exchange(true);

    if (!already_completed) {
        latch.count_down();
    }
    else {
        /**
         * TODO: handle this case, could happen in real life.
         * Probably, we can just ignore.
        */
        assert(!already_completed);
    }
}

// void ClientRequestInfo::add_mapper_job(MapRequest const& request) {
//     std::lock_guard<std::mutex> lock(mutex);
//     ongoing_map_requests[request.id()] = request;
// }

// void ClientRequestInfo::add_reducer_job(ReduceRequest const& request) {
//     std::lock_guard<std::mutex> lock(mutex);
//     ongoing_reduce_requests[request.id()] = request;
// }

// void ClientRequestInfo::complete_mapper_job(uint32_t id) {
//     std::lock_guard<std::mutex> lock(mutex);
//     ongoing_map_requests.erase(id);
//     map_latch.count_down();
// }

// void ClientRequestInfo::complete_reducer_job(uint32_t id) {
//     std::lock_guard<std::mutex> lock(mutex);
//     ongoing_reduce_requests.erase(id);
//     reduce_latch.count_down();
// }

// void ClientRequestInfo::wait_for_map() {
//     map_latch.wait();
// }

// void ClientRequestInfo::wait_for_reduce() {
//     reduce_latch.wait();
// }

// ClientRequestInfo& ClientRequestQueue::add_request(uint32_t id, ClientRequest const& request) {
//     std::lock_guard<std::mutex> lock(mutex);
//     assert(ongoing_requests.find(id) == ongoing_requests.end());
//     std::cerr << "Adding request with id: " << id << std::endl;
//     return ongoing_requests.emplace(id, request).first->second;
// }

// void ClientRequestQueue::complete_request(uint32_t id) {
//     std::lock_guard<std::mutex> lock(mutex);
//     assert(ongoing_requests.find(id) != ongoing_requests.end());
//     ongoing_requests.erase(id);
// }

// ClientRequestInfo& ClientRequestQueue::get_request_info(uint32_t id) {
//     std::lock_guard<std::mutex> lock(mutex);
//     auto it = ongoing_requests.find(id);
//     assert(it != ongoing_requests.end());
//     return it->second;
// }

} // mapreduce
