#ifndef HEALTH_CHECKER_CC
#define HEALTH_CHECKER_CC

#include "health-checker.h"
#include "cloud-utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace mapreduce {

void WorkerHealthStatus::report_health_check(HealthStatus reported_status)
{
    if (reported_status == UNHEALTHY) {
        retries++;
        if (retries >= retries_threshold) {
            status = UNHEALTHY;
        }
    } else {
        status = HEALTHY;
        retries = 0;
    }
}

HealthStatus HealthMap::get_status(std::string ip)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!worker_to_status.contains(ip)) {
        worker_to_status[ip] = WorkerHealthStatus();
    }
    return worker_to_status[ip].get_status();
}

void HealthMap::set_status(std::string ip, HealthStatus status)
{
    std::lock_guard<std::mutex> lock(mutex);
    worker_to_status[ip].report_health_check(status);
}

std::vector<std::string> HealthMap::get_workers()
{
    std::vector<std::string> workers;
    std::lock_guard<std::mutex> lock(mutex);

    for (auto& [worker, _] : worker_to_status) {
        workers.push_back(worker);
    }

    return workers;
}

void HealthChecker::update_worker_status(std::string ip, HealthStatus status)
{
    monitored_workers.set_status(ip, status);
}

HealthStatus HealthChecker::get_status(std::string ip)
{
    return monitored_workers.get_status(ip);
}

HealthStatus HealthChecker::check(std::string ip)
{
    // log_message("[HEALTH CHECKER] Sending health check to worker: " + ip + ".");

    std::unique_ptr<Health::Stub> health_stub(Health::NewStub(
        grpc::CreateChannel(ip, grpc::InsecureChannelCredentials())));

    HealthCheckResponse response;
    grpc::ClientContext context;
    std::chrono::time_point deadline = std::chrono::system_clock::now()
        + std::chrono::milliseconds(timeout_ms);
    context.set_deadline(deadline);

    HealthCheckRequest hc_request;

    grpc::Status status = health_stub->Check(&context, hc_request, &response);

    HealthStatus health_status = UNHEALTHY;
    if (status.ok()) {
        // log_message("[HEALTH CHECKER] Response from " + ip + " received.");
        health_status = response.status();
    }

    // log_message("[HEALTH CHECKER] Health status of " + ip + ": "
    //     + std::to_string(health_status));

    return health_status;
}

void HealthChecker::start()
{
    log_message("[HEALTH CHECKER] Started.");
    while (true) {
        std::vector<std::string> worker_list = monitored_workers.get_workers();

        for (std::string worker_ip : worker_list) {
            HealthStatus status = check(worker_ip);
            update_worker_status(worker_ip, status);
            // TODO: manage retries and removals of unhealthy workers.
        }

        std::this_thread::sleep_for(std::chrono::seconds(sleep_duration_s));
    }
}

} // mapreduce

#endif // HEALTH_CHECKER_CC