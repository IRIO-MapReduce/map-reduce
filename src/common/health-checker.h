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

class HealthMap {
public:
    HealthStatus get_status(std::string worker);
    void set_status(std::string ip, HealthStatus status);

    std::vector<std::string> get_workers();

private:
    std::unordered_map<std::string, HealthStatus> worker_to_status;
    std::mutex mutex;
};

class HealthChecker {
public:
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
    int timeout_ms = 500; // 0.5s
    int sleep_duration_s = 3; // 3s

    void update_worker_status(std::string ip, HealthStatus status);

    /**
     * Sends gRPC request to the ip and returns status of the machine.
    */
    HealthStatus check(std::string ip);
};

} // mapreduce

#endif // HEALTH_CHECKER_H
