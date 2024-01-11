#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <string>
#include <vector>

namespace mapreduce {

constexpr size_t DEFAULT_SPLIT_SIZE_MB = 32;

const std::string MASTER_ADDRESS = "0.0.0.0:50051";
const std::string MAPPER_ADDRESS = "0.0.0.0:50052";
const std::string REDUCER_ADDRESS = "0.0.0.0:50053";

const std::string MAPPER_LISTENER_ADDRESS = "mapper-service:50054";
const std::string REDUCER_LISTENER_ADDRESS = "0.0.0.0:50055";

class Config {
public:
    inline void set_input_filepath(std::string const& filepath) noexcept {
        this->input_filepath = filepath;
    }

    inline void set_output_filepath(std::string const& filepath) noexcept {
        this->output_filepath = filepath;
    }

    inline void set_mapper_execpath(std::string const& execpath) noexcept {
        this->mapper_execpath = execpath;
    }

    inline void set_reducer_execpath(std::string const& execpath) noexcept {
        this->reducer_execpath = execpath;
    }

    inline void set_split_size_bytes(size_t split_size_bytes) noexcept {
        this->split_size_bytes = split_size_bytes;
    }

    inline void set_split_size_kb(size_t split_size_kb) noexcept { 
        set_split_size_bytes(split_size_kb * 1024);
    }

    inline void set_split_size_mb(size_t split_size_mb) noexcept {
        set_split_size_bytes(split_size_mb * 1024 * 1024);
    }

    inline void set_split_size(size_t split_size_mb) noexcept {
        set_split_size_mb(split_size_mb);
    }

    inline void set_num_reducers(uint32_t num_reducers) noexcept {
        this->num_reducers = num_reducers;
    }

private:
    std::string input_filepath;
    std::string output_filepath;
    std::string mapper_execpath;
    std::string reducer_execpath;
    size_t split_size_bytes;
    uint32_t num_reducers;

    friend void map_reduce(Config const& config);
};

void map_reduce(Config const& config);

} // mapreduce

#endif // MAP_REDUCE_H