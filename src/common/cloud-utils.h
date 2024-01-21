#ifndef CLOUD_UTILS_H
#define CLOUD_UTILS_H

#include <google/cloud/logging/v2/logging_service_v2_client.h>
#include <optional>
#include <string>
#include <vector>

namespace mapreduce {

const std::string PROJECT_ID = "pb-map-reduce";
const std::string NETWORK_NAME = "nic0";
const std::string WORKER_PREFIX = "worker-";
const std::string MASTER_PREFIX = "master-";
const std::string LOCALHOST = "127.0.0.1";
const std::string LISTENING_ADDRESS = "0.0.0.0";
const uint16_t MASTER_PORT = 50051;
const uint16_t JOB_MANAGER_PORT = 50052;
const uint16_t WORKER_PORT = 50053;
const uint16_t HEALTH_CHECK_PORT = 50054;

/**
 * Returns a list of all currently available workers' IPs.
 */
std::vector<std::string> get_worker_ips();

/**
 * Returns the IP of the master node.
 */
std::optional<std::string> get_master_ip();

inline std::string get_address(std::string const& ip, uint16_t port)
{
    return ip + ":" + std::to_string(port);
}

std::string uri_to_url(std::string const& address);

/**
 * Logs a message to the Google Cloud Logging service.
 * @param message The message to log.
 * @param severity The severity of the message. One of the following:
 *  1. DEFAULT
 *  2. DEBUG
 *  3. INFO
 *  4. NOTICE
 *  5. WARNING
 *  6. ERROR
 *  7. CRITICAL
 *  8. ALERT
 *  9. EMERGENCY
 * @param name The name of the log to write to.
 * @param resoure_type The type of resource. Should be one of specific,
 * described at: https://cloud.google.com/monitoring/api/resources. The last
 * two parameters are optional for future use and not currently widely used.
 */
void log_message(std::string const& message,
    google::logging::type::LogSeverity severity
    = google::logging::type::LogSeverity::DEFAULT,
    std::map<std::string, std::string> const& labels = {},
    std::string const& name = "mapreduce",
    std::string const& resource_type = "global");

} // namespace mapreduce

#endif // CLOUD_UTILS_H