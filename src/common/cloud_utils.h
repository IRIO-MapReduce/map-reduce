#ifndef CLOUD_UTILS_H
#define CLOUD_UTILS_H

#include <string>
#include <vector>

namespace mapreduce {

const std::string NETWORK_NAME = "nic0";
const std::string WORKER_PREFIX = "mapper-"; // [TODO] Change to "worker-" after change on cloud
const std::string PROJECT_ID = "pb-map-reduce";

std::vector<std::string> get_worker_ips();

} // namespace

#endif // CLOUD_UTILS_H