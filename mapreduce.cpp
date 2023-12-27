#include <fstream>
#include <iostream>

#include "mapreduce.h"
#include "utils.h"

namespace mapreduce {
    
void Worker::emit(key_t const& key, value_t const& value) const {
    std::ofstream output_file(output_filepath, std::ios_base::app);
    output_file << key << "," << value << '\n';
}

void Worker::set_emit_file(std::string const& filepath) {
    this->output_filepath = filepath;
}

void Config::set_input_file(std::string const& filepath) {
    this->input_filepath = filepath;
}

void Config::set_output_file(std::string const& filepath) {
    this->output_filepath = filepath;
}

void Config::set_mapper(Mapper *mapper) {
    this->mapper = mapper;
}

void Config::set_reducer(Reducer *reducer) {
    this->reducer = reducer;
}

void map_reduce(Config const& config) {
    auto *mapper = config.mapper;
    auto *reducer = config.reducer;

    std::string intermediate_filepath = config.input_filepath;
    intermediate_filepath.erase(intermediate_filepath.length() - 4);    
    intermediate_filepath += INTERMEDIATE_FILE_SUFFIX;

    std::cerr << "Starting map phase...\n";
    std::cerr << config.input_filepath << " -> " << intermediate_filepath << '\n';

    mapper->set_emit_file(intermediate_filepath);
    mapper->map(config.input_filepath);

    std::cerr << "Starting reduce phase...\n";
    std::cerr << intermediate_filepath << " -> " << config.output_filepath << '\n';

    reducer->set_emit_file(config.output_filepath);
    reducer->reduce(intermediate_filepath);

    std::cerr << "Done!\n";
}

} // mapreduce