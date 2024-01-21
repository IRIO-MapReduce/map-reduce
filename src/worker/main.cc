#include <thread>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"
#include "../common/cloud-utils.h"
#include "health-check-service.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;

using namespace mapreduce;

class WorkerServiceImpl final : public Worker::Service {
public:
    Status ProcessJobRequest(ServerContext* context, const JobRequest* request, Response* response) override {
        log_message("[WORKER LISTENER] Received JobRequest.", 
                    google::logging::type::LogSeverity::DEFAULT,{
                        {"group_id", std::to_string(request->group_id())},
                        {"job_id", std::to_string(request->job_id())},
        });
        
        // Save config for later
        request_queue.add(*request);

        // Run mapper binary
        std::system(std::string(request->execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetFreeTask(ServerContext* context, const ConfigRequest* request, JobRequest* job_request) override {
        log_message("[REDUCER LISTENER] Config request.");
        
        // Respond with saved config.
        *job_request = request_queue.get_one(request->execpath());

        return Status::OK;
    }

    void get_job_manager_address() {
        while (true) {
            log_message("[WORKER] Trying to acquire job manager address...");
            auto job_manager_ip = get_master_ip();
            
            if (job_manager_ip.has_value()) {
                job_manager_address = get_address(job_manager_ip.value(), JOB_MANAGER_PORT);
                log_message("[WORKER] Job manager address acquired: " + job_manager_address);
                return;
            }

            log_message("[WORKER] Job manager not found. Retrying in 5 seconds...", 
                        google::logging::type::LogSeverity::WARNING);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }


private:
    WorkQueue request_queue;
    std::string job_manager_address;
};

void RunWorkerServer() {
    std::string server_address(get_address(LISTENING_ADDRESS, WORKER_PORT));
    WorkerServiceImpl service;

    service.get_job_manager_address();

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    log_message("[WORKER] Server listening on " + server_address);

    server->Wait();
}

int main() {
    HealthCheckServiceImpl health_service(get_address(LISTENING_ADDRESS, HEALTH_CHECK_PORT));
    std::thread health_service_thread(&HealthCheckServiceImpl::start, &health_service);

    RunWorkerServer();

    health_service_thread.join();
    return 0;
}