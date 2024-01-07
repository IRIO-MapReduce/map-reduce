#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <grpc++/grpc++.h>
#include "mapreduce.grpc.pb.h"

#include "../common/mapreduce.h"
#include "../common/utils.h"

class CounterMapper : public mapreduce::Mapper {
public:
    void map(std::string const& filepath) const override {
        std::ifstream file(filepath);
        std::string line;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string key, value;

            std::getline(ss, key, ',');
            std::getline(ss, value);

            emit(key, "1");
        }
    }
};

class CounterReducer : public mapreduce::Reducer {
public:
    void reduce(std::vector<std::string> const& filepaths) const override {
        std::unordered_map<std::string, int> counts;

        for (auto const& filepath : filepaths) {
            std::ifstream file(filepath);
            std::string line;
            
            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string key, value;

                std::getline(ss, key, ',');
                std::getline(ss, value);

                counts[key] += std::stoi(value);
            }
        }
        
        for (auto const& [key, value] : counts) {
            emit(key, std::to_string(value));
        }
    }
};

class MapReduceServiceImpl final : public mapreduce::Server::Service {
public:
    grpc::Status MapReduce(grpc::ServerContext* context, const mapreduce::Request* request, mapreduce::Reply* reply) override {
        std::string input_path = "/fs/" + request->filename();
        std::string output_path("/fs/output.txt");

        std::cerr << "MapReduce request received, file: " << input_path << std::endl;
        std::cerr << "Output file set to: " << output_path << std::endl;

        mapreduce::Config config;
        config.set_input_file(input_path);
        config.set_output_file(output_path);

        std::string prefix("Output saved in: ");
        reply->set_filename(prefix + output_path);

        CounterMapper mapper;
        CounterReducer reducer;

        config.set_mapper(&mapper);
        config.set_reducer(&reducer);

        config.set_split_size(1);

        std::cerr << "MapReduce starting" << std::endl;

        mapreduce::map_reduce(config);

        std::cerr << "MapReduce finished" << std::endl;

        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    MapReduceServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}