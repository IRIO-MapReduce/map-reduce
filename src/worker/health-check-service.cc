#ifndef HEALTH_CHECKER_CC
#define HEALTH_CHECKER_CC

#include "health-check-service.h"
#include "cloud-utils.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace mapreduce {

Status HealthCheckServiceImpl::Check(ServerContext* context,
    const HealthCheckRequest* request, HealthCheckResponse* response)
{
    log_message("[HEALTH CHECK SERVICE] Received Health Check.");

    response->set_status(HEALTHY);

    return Status::OK;
}

void HealthCheckServiceImpl::start()
{
    ServerBuilder builder;
    builder.AddListeningPort(
        service_address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    log_message(
        "[HEALTH CHECK SERVICE] Service listening on " + service_address);

    server->Wait();
}

} // mapreduce

#endif // HEALTH_CHECKER_CC
