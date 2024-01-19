#include <google/cloud/compute/instances/v1/instances_client.h>

#include "cloud_utils.h"

namespace mapreduce {

static inline bool matches_prefix(const std::string& str, const std::string& prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

static std::vector<std::string> get_list_ips(const std::string& prefix) try {
    namespace instances = ::google::cloud::compute_instances_v1;
    auto client = instances::InstancesClient(instances::MakeInstancesConnectionRest());

    std::vector<std::string> result;

    for (auto zone : client.AggregatedListInstances(PROJECT_ID)) {
        if (!zone) throw std::move(zone).status();
        else {
            auto const& instance_list = zone->second.instances();
            for (auto const& instance : instance_list) {
                if (!matches_prefix(instance.name(), prefix)) continue;
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

std::vector<std::string> get_worker_ips() {
    return {LOCALHOST};
    // return get_list_ips(WORKER_PREFIX);
}

std::optional<std::string> get_master_ip() {
    return LOCALHOST;
    // auto ips = get_list_ips(MASTER_PREFIX);
    // assert(ips.size() <= 1);
    // if (ips.empty()) {
    //     return std::nullopt;
    // }
    // return ips.back();
}

} // mapreduce