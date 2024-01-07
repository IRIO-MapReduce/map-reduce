#include <stdexcept>
#include <fstream>
#include <iostream>

#include "utils.h"

namespace mapreduce {

size_t split_file(std::string const& filepath, size_t part_size_mb) {
    // 1 KB overhead for splitting lines
    static constexpr size_t MAX_LINE_SIZE_BYTES = 1024;

    if (part_size_mb == 0) {
        throw std::invalid_argument("part_size_mb must be greater than 0");
    }

    std::ifstream file(filepath, std::ios::ate);
    if (!file.is_open()) {
        throw std::invalid_argument("could not open file");
    }

    size_t part_size_bytes = part_size_mb * 1024 * 1024;
    char* buffer = new char[part_size_bytes + MAX_LINE_SIZE_BYTES];

    std::streampos end_pos = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t num_parts;
    for (num_parts = 0; file.tellg() < end_pos; num_parts++) {
        std::streampos cur_start = file.tellg();
        std::streamoff cur_end = cur_start + static_cast<std::streamoff>(part_size_bytes);

        if (cur_end >= end_pos) {
            file.seekg(0, std::ios::end);
            cur_end = end_pos;
        }
        else {
            std::string temp;
            file.seekg(cur_end);

            if (!getline(file, temp)) {
                delete[] buffer;
                throw std::runtime_error("error while cropping lines");
            }

            if (file.eof()) {
                cur_end = end_pos;
            }
            else {
                cur_end = file.tellg();
            }
        }

        std::string split_filepath = get_split_filepath(filepath, num_parts);
        std::ofstream split_file(split_filepath, std::ios::binary);

        if (!split_file.is_open()) {
            delete[] buffer;
            throw std::invalid_argument("could not create file");
        }

        file.seekg(cur_start);
        size_t num_bytes = cur_end - cur_start;

        // In case last line is particularly long, allocate extra buffer.
        if (num_bytes > part_size_bytes + MAX_LINE_SIZE_BYTES) {
            char* overhead_buffer = new char[num_bytes];
            file.read(overhead_buffer, num_bytes);
            split_file.write(overhead_buffer, num_bytes);
            delete[] overhead_buffer;
        }
        else {
            file.read(buffer, num_bytes);
            split_file.write(buffer, num_bytes);
        }
    }

    delete[] buffer;
    return num_parts;
}

bool has_valid_format(std::string const& filepath) {
    return filepath.length() >= 4 && filepath.substr(filepath.length() - 4) == ".txt";
}

std::string get_intermediate_filepath(std::string const& filepath) {
    std::string intermediate_filepath = filepath;
    intermediate_filepath.erase(intermediate_filepath.length() - 4);    
    intermediate_filepath += INTERMEDIATE_FILE_SUFFIX;

    return intermediate_filepath;
}

std::string get_split_filepath(std::string const& filepath, size_t split_index) {
    std::string split_filepath = filepath;
    split_filepath.erase(split_filepath.length() - 4);
    split_filepath += "-split-" + std::to_string(split_index) + ".txt";

    return split_filepath;
}

} // mapreduce