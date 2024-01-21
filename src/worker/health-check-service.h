#ifndef HEALTH_CHECKER_H
#define HEALTH_CHECKER_H

#include <chrono>
#include <fstream>
#include <grpc++/grpc++.h>
#include <string>
#include <thread>
#include <vector>

#include "mapreduce.grpc.pb.h"
#include "utils.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace mapreduce {

class HealthCheckServiceImpl final : public Health::Service {
public:
    HealthCheckServiceImpl(const std::string& service_address)
        : service_address(service_address)
    {
    }

    void start();

    Status Check(ServerContext* context, const HealthCheckRequest* request,
        HealthCheckResponse* response) override;

private:
    std::string service_address;
};

} // mapreduce

#endif // HEALTH_CHECKER_H
