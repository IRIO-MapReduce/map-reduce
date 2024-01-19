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

} // mapreduce
