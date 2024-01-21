#include <iostream>
#include <shared_mutex>
#include <thread>

#include "cloud-utils.h"
#include "health-checker.h"
#include "load-balancer.h"

namespace mapreduce {

static constexpr uint32_t MAX_WORKER_THREADS = 4;

static inline void notify_many(std::condition_variable_any& cv, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        cv.notify_one();
}

static inline uint32_t thread_safe_rand()
{
    static std::atomic<bool> lock;
    while (!lock.exchange(true, std::memory_order_acquire))
        std::this_thread::yield();

    uint32_t val = rand();
    lock.store(false, std::memory_order_release);
    return val;
}

void LoadBalancer::release_worker(uint32_t idx)
{
    uint32_t val;
    while ((val = busy_threads[idx]) > 0) {
        if (busy_threads[idx].compare_exchange_weak(
                val, val - 1, std::memory_order_release)) {
            available_workers++;
            cv.notify_one();
            return;
        }
    }
}

bool LoadBalancer::try_acquire_worker()
{
    uint32_t val;
    while ((val = available_workers) > 0) {
        if (available_workers.compare_exchange_weak(
                val, val - 1, std::memory_order_acquire))
            return true;
    }
    return false;
}

bool LoadBalancer::try_acquire_worker(uint32_t idx)
{
    uint32_t val = busy_threads[idx];
    return val < MAX_WORKER_THREADS
        && busy_threads[idx].compare_exchange_weak(
            val, val + 1, std::memory_order_acquire);
}

std::string LoadBalancer::get_worker_ip_unchecked()
{
    std::shared_lock lock(mutex);
    cv.wait(lock, [this]() { return try_acquire_worker(); });

    auto idx = thread_safe_rand() % busy_threads.size();
    while (!try_acquire_worker(idx)) {
        if (++idx == busy_threads.size())
            idx = 0;
    }

    return worker_ips[idx];
}

std::string LoadBalancer::get_worker_ip()
{
    while (true) {
        auto worker_ip = get_worker_ip_unchecked();
        auto health_check_address = get_address(worker_ip, HEALTH_CHECK_PORT);

        if (health_checker.get_status(health_check_address)
            == HealthStatus::HEALTHY)
            return worker_ip;

        {
            std::unique_lock lock(mutex);

            if (std::find(unhealthy_workers.begin(), unhealthy_workers.end(),
                    worker_ip)
                != unhealthy_workers.end())
                continue;

            unhealthy_workers.push_back(worker_ip);
        }

        log_message(
            "[LOAD BALANCER] Worker " + worker_ip + " marked as unhealthy!",
            google::logging::type::WARNING);

        // Preventively, wait for some time before requesting again, to not spam unhealthy worker.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void LoadBalancer::notify_worker_finished(std::string const& worker_ip)
{
    std::shared_lock lock(mutex);
    auto idx = idx_of_worker[worker_ip];
    release_worker(idx);
}

void LoadBalancer::start()
{
    std::thread health_checker_thread([this]() { health_checker.start(); });

    while (true) {
        refresh_workers();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        check_unhealthy_workers();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    health_checker_thread.join();
}

void LoadBalancer::check_unhealthy_workers()
{
    std::vector<std::string> new_unhealthy_workers;
    std::unique_lock lock(mutex);

    for (auto const& worker_ip : unhealthy_workers) {
        auto health_check_address = get_address(worker_ip, HEALTH_CHECK_PORT);
        if (health_checker.get_status(health_check_address)
            == HealthStatus::HEALTHY) {
            log_message(
                "[LOAD BALANCER] Marking worker as healthy: " + worker_ip,
                google::logging::type::INFO);

            auto idx = idx_of_worker[worker_ip];
            release_worker(idx);
        } else {
            new_unhealthy_workers.push_back(worker_ip);
        }
    }

    unhealthy_workers = new_unhealthy_workers;
}

void LoadBalancer::refresh_workers()
{
    auto new_worker_ips = get_worker_ips();
    std::unordered_map<std::string, uint32_t> new_idx_of_worker;
    std::vector<uint32_t> old_busy_threads(busy_threads.size());

    std::unique_lock lock(mutex);

    for (uint32_t i = 0; i < busy_threads.size(); i++)
        old_busy_threads[i] = busy_threads[i];

    busy_threads = std::vector<std::atomic<uint32_t>>(new_worker_ips.size());

    for (uint32_t i = 0; i < new_worker_ips.size(); i++) {
        auto const& worker_ip = new_worker_ips[i];
        new_idx_of_worker[worker_ip] = i;

        auto it = idx_of_worker.find(worker_ip);
        if (it == idx_of_worker.end()) {
            log_message("[LOAD BALANCER] Found new worker: " + worker_ip,
                google::logging::type::INFO);

            busy_threads[i] = 0;
            available_workers += MAX_WORKER_THREADS;
            notify_many(cv, MAX_WORKER_THREADS);
        } else {
            auto old_val = old_busy_threads[it->second];

            // Here the policy periodically adds more tokens to VMs, so that
            // jobs that failed without notice of health check are
            // garbage-collected.
            if (old_val > 0) {
                busy_threads[i] = old_val - 1;
                available_workers++;
                cv.notify_one();
            } else {
                busy_threads[i] = old_val;
            }
        }
    }

    cv.notify_all();

    worker_ips = new_worker_ips;
    idx_of_worker = new_idx_of_worker;
}

} // mapreduce