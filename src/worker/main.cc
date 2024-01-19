#include <thread>
#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"
#include "../common/cloud_utils.h"
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
        std::cerr << "[WORKER LISTENER] Received JobRequest." << std::endl;
        std::cerr << " ---------------  group_id: " << request->group_id() << std::endl;
        std::cerr << " ---------------  job_id: " << request->job_id() << std::endl;
        
        // Save config for later
        request_queue.add(*request);

        // Run mapper binary
        std::system(std::string(request->execpath() + " &").c_str());

        return Status::OK;
    }

    Status GetFreeTask(ServerContext* context, const ConfigRequest* request, JobRequest* job_request) override {
        std::cerr << "[REDUCER LISTENER] Received ReduceConfig request." << std::endl;
        
        // Respond with saved config.
        *job_request = request_queue.get_one(request->execpath());

        return Status::OK;
    }

    void get_job_manager_address() {
        while (true) {
            std::cerr << "[WORKER] Trying to acquire job manager address..." << std::endl;
            auto job_manager_ip = get_master_ip();
            
            if (job_manager_ip.has_value()) {
                job_manager_address = get_address(job_manager_ip.value(), JOB_MANAGER_PORT);
                std::cerr << "[WORKER] Job manager address acquired: " << job_manager_address << std::endl;
                return;
            }

            std::cerr << "[WORKER] Job manager not found. Retrying in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }


private:
    WorkQueue request_queue;
    std::string job_manager_address;
};

void RunWorkerServer() {
    std::string server_address(get_address(LOCALHOST, WORKER_PORT));
    WorkerServiceImpl service;

    service.get_job_manager_address();

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[WORKER] Server listening on " << server_address << std::endl;

    server->Wait();
}

int main() {
    HealthCheckServiceImpl health_service("0.0.0.0:50056");
    std::thread health_service_thread(&HealthCheckServiceImpl::start, &health_service);

    RunWorkerListenerServer();

    health_service_thread.join();
    return 0;
}