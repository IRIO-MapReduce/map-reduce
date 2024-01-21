#include <google/cloud/compute/instances/v1/instances_client.h>
#include <google/cloud/logging/v2/logging_service_v2_client.h>

#include "cloud-utils.h"

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
#ifdef LOCAL
    return {LOCALHOST};
#else
    return get_list_ips(WORKER_PREFIX);
#endif
}

std::optional<std::string> get_master_ip() {
#ifdef LOCAL
    return LOCALHOST;
#else
    auto ips = get_list_ips(MASTER_PREFIX);
    assert(ips.size() <= 1);
    if (ips.empty()) {
        return std::nullopt;
    }
    return ips.back();
#endif
}

/**
 * TODO: Verify and rewrite.
*/
std::string uri_to_url(std::string const& address) {
    size_t pos = address.find(':');
    assert (pos != std::string::npos);
    auto result = address.substr(pos + 1);
    pos = result.find(':');
    assert (pos != std::string::npos);
    result = result.substr(0, pos);
    return result;
}

void log_message(
    std::string const& message, 
    google::logging::type::LogSeverity severity,
    std::map<std::string, std::string> const& labels,
    std::string const& name,
    std::string const& resource_type
) {
    namespace logging = ::google::cloud::logging_v2;
    auto client = logging::LoggingServiceV2Client(logging::MakeLoggingServiceV2Connection());

    auto request = google::logging::v2::WriteLogEntriesRequest();
    auto log_entry = request.add_entries();
    auto log_name = "projects/" + PROJECT_ID + "/logs/" + name;
    
    request.set_log_name(log_name);

    log_entry->set_text_payload(message);
    log_entry->set_severity(severity);
    log_entry->mutable_resource()->set_type(resource_type);

    for (auto const& [key, value] : labels) {
        (*log_entry->mutable_labels())[key] = value;
    }

    auto response = client.WriteLogEntries(request);

    if (!response.ok()) {
        std::cerr << "Failed to log message: " << response.status().message() << std::endl;
    }
}

} // mapreduce
