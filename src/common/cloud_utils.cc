#include <google/cloud/compute/instances/v1/instances_client.h>

#include "cloud_utils.h"

namespace mapreduce {

static inline bool is_worker_instance(const std::string& instance_name) {
    return instance_name.substr(0, WORKER_PREFIX.size()) == WORKER_PREFIX;
}

std::vector<std::string> get_worker_ips() try {
    namespace instances = ::google::cloud::compute_instances_v1;
    auto client = instances::InstancesClient(instances::MakeInstancesConnectionRest());

    std::vector<std::string> result;

    for (auto zone : client.AggregatedListInstances(PROJECT_ID)) {
        if (!zone) throw std::move(zone).status();
        else {
            auto const& instance_list = zone->second.instances();
            for (auto const& instance : instance_list) {
                if (!is_worker_instance(instance.name())) continue;
                for (auto const& network : instance.network_interfaces()) {
                    if (network.name() == NETWORK_NAME) {
                        result.push_back(network.network_ip());
                        break;
                    }
                }
            }
        }
    }

    return result;
} catch (google::cloud::Status const& status) {
    assert (false);
    return {};
}

} // mapreduce