#include <grpc++/grpc++.h>

#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"
#include "../common/data-structures.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;

using namespace mapreduce;

class WorkerServiceImpl final : public Worker::Service {
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

private:
    WorkQueue request_queue;
};

void RunWorkerServer() {
    std::string server_address(WORKER_ADDRESS);
    WorkerServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cerr << "[WORKER] Server listening on " << server_address << std::endl;

    server->Wait();
}

int main() {
    RunWorkerServer();
    return 0;
}