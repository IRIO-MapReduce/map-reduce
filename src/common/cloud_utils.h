#ifndef CLOUD_UTILS_H
#define CLOUD_UTILS_H

#include <string>
#include <vector>
#include <optional>

namespace mapreduce {

const std::string PROJECT_ID = "pb-map-reduce";
const std::string NETWORK_NAME = "nic0";
const std::string WORKER_PREFIX = "worker-";
const std::string MASTER_PREFIX = "master-";
const std::string LOCALHOST = "0.0.0.0";
const uint16_t MASTER_PORT = 50051;
const uint16_t JOB_MANAGER_PORT = 50052;
const uint16_t WORKER_PORT = 50053;

/**
 * Returns a list of all currently available workers' IPs.
*/
std::vector<std::string> get_worker_ips();

/**
 * Returns the IP of the master node.
*/
std::optional<std::string> get_master_ip();

inline std::string get_address(std::string const& ip, uint16_t port) {
    return ip + ":" + std::to_string(port);
}

} // mapreduce

#endif // CLOUD_UTILS_H