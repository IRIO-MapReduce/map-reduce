#ifndef HEALTH_CHECKER_H
#define HEALTH_CHECKER_H

#include <vector>
#include <string>
#include <fstream>
#include<chrono>
#include <thread>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"
#include "utils.h"


namespace mapreduce {

class WorkerHealthStatus {
public:
    WorkerHealthStatus() : status(UNHEALTHY), retries(0), retries_threshold(3) {}
    void report_health_check(HealthStatus reported_status);
    HealthStatus get_status() {
        return status;
    }
private:
    HealthStatus status;
    uint16_t retries, retries_threshold;
};

class HealthMap {
public:
    HealthStatus get_status(std::string worker);
    void set_status(std::string ip, HealthStatus status);

    std::vector<std::string> get_workers();

private:
    std::unordered_map<std::string, WorkerHealthStatus> worker_to_status;
    std::mutex mutex;
};

class HealthChecker {
public:
    /**
     * By default health checker sets health check timeout to 0.5s 
     * and sleep duration between health check phases to 3s.
    */
    HealthChecker(int timeout_ms = 500, int sleep_duration_s = 5) : timeout_ms(timeout_ms), sleep_duration_s(sleep_duration_s) {}

    /**
     * Returns health status of a machine with given ip.
    */
    HealthStatus get_status(std::string ip);

    /**
     * Starts health check service.
    */
    void start();
private:
    HealthMap monitored_workers;
    int timeout_ms;
    int sleep_duration_s;

    void update_worker_status(std::string ip, HealthStatus status);

    /**
     * Sends gRPC request to the ip and returns status of the machine.
    */
    HealthStatus check(std::string ip);
};

} // mapreduce

#endif // HEALTH_CHECKER_H
