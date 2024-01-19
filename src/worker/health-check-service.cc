#ifndef HEALTH_CHECKER_CC
#define HEALTH_CHECKER_CC

#include "health-check-service.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace mapreduce {

Status HealthCheckServiceImpl::Check(ServerContext* context, const HealthCheckRequest* request, HealthCheckResponse* response) {
    std::cerr << "[HEALTH CHECK SERVICE] Received Health Check." << std::endl;

    response->set_status(HEALTHY);
    
    return Status::OK;
}

HealthCheckServiceImpl::HealthCheckServiceImpl(const std::string &service_address) {
    Health::Service();
    ServerBuilder builder;
    builder.AddListeningPort(service_address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[HEALTH CHECK SERVICE] Service listening on " << service_address << std::endl;

    server->Wait();
}

} // mapreduce

#endif // HEALTH_CHECKER_CC
