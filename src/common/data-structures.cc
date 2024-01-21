#include "data-structures.h"

namespace mapreduce {

void WorkQueue::add(JobRequest const& item)
{
    std::lock_guard<std::mutex> lock(mutex);
    work_queue[item.execpath()].push(item);
}

JobRequest WorkQueue::get_one(std::string const& execpath)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto& queue = work_queue[execpath];
    assert(!queue.empty());
    JobRequest item = queue.front();
    queue.pop();
    return item;
}

void JobGroup::add_job(JobRequest const& request)
{
    uint32_t id = request.job_id();
    jobs[id] = request;
}

bool JobGroup::wait_for_completion(uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(mutex);
    bool finished_successfully = cv.wait_for(lock,
        std::chrono::seconds(timeout), [this]() { return num_jobs == 0; });

    return finished_successfully;
}

std::vector<JobRequest> JobGroup::get_unfinished_jobs()
{
    std::vector<JobRequest> result;
    for (uint32_t i = 0; i < jobs.size(); ++i) {
        if (!completed[i]) {
            result.push_back(jobs[i]);
        }
    }
    return result;
}

void JobGroup::mark_completed(uint32_t id)
{
    bool already_completed = completed[id].exchange(true);

    if (!already_completed) {
        if (--num_jobs == 0) {
            cv.notify_one();
        }
    } else {
        /**
         * TODO: handle this case, could happen in real life.
         * Probably, we can just ignore.
         */
    }
}

} // namespace mapreduce
