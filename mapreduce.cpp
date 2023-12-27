#include <fstream>
#include <iostream>

#include "mapreduce.h"
#include "utils.h"

namespace mapreduce {
    
void Worker::emit(key_t const& key, value_t const& value) const {
    std::ofstream output_file(output_filepath, std::ios_base::app);
    output_file << key << "," << value << '\n';
}

void Worker::set_emit_file(std::string const& filepath) { this->output_filepath = filepath; }

void Config::set_input_file(std::string const& filepath) { this->input_filepath = filepath; }

void Config::set_output_file(std::string const& filepath) { this->output_filepath = filepath; }

void Config::set_mapper(Mapper *mapper) { this->mapper = mapper; }

void Config::set_reducer(Reducer *reducer) { this->reducer = reducer; }

void Config::set_split_size(size_t split_size_mb) { this->split_size_mb = split_size_mb; }

void map_reduce(Config const& config) {
    auto *mapper = config.mapper;
    auto *reducer = config.reducer;

    size_t num_parts = split_file(config.input_filepath, config.split_size_mb);

    std::cerr << "Starting map phase...\n";

    std::vector<std::string> intermediate_filepaths;

    for (size_t i = 0; i < num_parts; i++) {
        std::string split_filepath = get_split_filepath(config.input_filepath, i);
        std::string intermediate_filepath = get_intermediate_filepath(split_filepath);

        intermediate_filepaths.push_back(intermediate_filepath);

        std::cerr << split_filepath << " -> " << intermediate_filepath << '\n';

        mapper->set_emit_file(intermediate_filepath);
        mapper->map(split_filepath);
    }

    std::cerr << "Starting reduce phase...\n";
    
    reducer->set_emit_file(config.output_filepath);
    reducer->reduce(intermediate_filepaths);

    std::cerr << "Done!\n";
}

} // mapreduce