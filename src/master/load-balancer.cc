#include <iostream>
#include <shared_mutex>
#include <thread>

#include "cloud-utils.h"
#include "health-checker.h"
#include "load-balancer.h"

namespace mapreduce {

static uint32_t thread_safe_rand()
{
    static std::atomic<bool> lock;
    while (!lock.exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    uint32_t val = rand();
    lock.store(false, std::memory_order_release);
    return val;
}

void LoadBalancer::release_worker(uint32_t idx)
{
    if (is_busy[idx].exchange(false, std::memory_order_release)) {
        available_workers++;
        cv.notify_one();
    }
}

bool LoadBalancer::try_acquire_worker()
{
    uint32_t val;
    while ((val = available_workers) > 0) {
        if (available_workers.compare_exchange_weak(val, val - 1))
            return true;
    }
    return false;
}

std::string LoadBalancer::get_worker_ip_unchecked()
{
    std::shared_lock lock(mutex);

    cv.wait(lock, [this]() { return try_acquire_worker(); });

    auto idx = thread_safe_rand() % is_busy.size();

    while (is_busy[idx].exchange(true, std::memory_order_acquire)) {
        if (++idx == is_busy.size()) {
            idx = 0;
        }
    }

    return worker_ips[idx];
}

std::string LoadBalancer::get_worker_ip()
{
    while (true) {
        auto worker_ip = get_worker_ip_unchecked();
        auto health_check_address = get_address(worker_ip, HEALTH_CHECK_PORT);
        // log_message(
        //     "[LOAD BALANCER] Checking worker health: " +
        //     health_check_address);
        if (health_checker.get_status(health_check_address)
            == HealthStatus::HEALTHY) {
            // log_message("[LOAD BALANCER] Found healthy worker: " +
            // worker_ip);
            return worker_ip;
        }

        // log_message("[LOAD BALANCER] Found unhealthy worker: " + worker_ip);

        std::unique_lock lock(mutex);
        unhealthy_workers.push_back(worker_ip);
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
        // log_message("[LOAD BALANCER] Refreshing workers...");
        refresh_workers();
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // log_message("[LOAD BALANCER] Checking unhealthy workers...");
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
                "[LOAD BALANCER] Marking worker as healthy: " + worker_ip);
            auto idx = idx_of_worker[worker_ip];
            release_worker(idx);
        } else {
            // log_message("[LOAD BALANCER] Worker still unhealthy: " +
            // worker_ip);
            new_unhealthy_workers.push_back(worker_ip);
        }
    }

    unhealthy_workers = new_unhealthy_workers;
}

void LoadBalancer::refresh_workers()
{
    auto new_worker_ips = get_worker_ips();
    std::unordered_map<std::string, uint32_t> new_idx_of_worker;
    std::vector<bool> old_is_busy(is_busy.size());

    std::unique_lock lock(mutex);

    for (uint32_t i = 0; i < is_busy.size(); i++) {
        old_is_busy[i] = is_busy[i].load();
    }

    is_busy = std::vector<std::atomic<bool>>(new_worker_ips.size());

    for (uint32_t i = 0; i < new_worker_ips.size(); i++) {
        auto const& worker_ip = new_worker_ips[i];
        auto it = idx_of_worker.find(worker_ip);
        new_idx_of_worker[worker_ip] = i;
        if (it == idx_of_worker.end()) {
            log_message("[LOAD BALANCER] Found new worker: " + worker_ip);
            available_workers++;
            cv.notify_one();
        } else {
            // log_message("[LOAD BALANCER] Found existing worker: " +
            // worker_ip);
            is_busy[i].store(false, std::memory_order_release);
            if (old_is_busy[it->second]) {
                available_workers++;
                cv.notify_one();
            }
        }
    }

    worker_ips = new_worker_ips;
    idx_of_worker = new_idx_of_worker;

    // log_message("[LOAD BALANCER] Refreshed workers.");
    // for (auto const& worker_ip : worker_ips) {
    //     log_message("\t[LOAD BALANCER] Worker: " + worker_ip);
    // }
}

} // mapreduce