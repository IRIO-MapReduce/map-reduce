#include <stdexcept>
#include <fstream>
#include <iostream>

#include "utils.h"

namespace mapreduce {

void split_file(std::string const& filepath, size_t part_size_mb) {
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

    std::streampos end_pos = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[part_size_bytes + MAX_LINE_SIZE_BYTES];

    for (size_t i = 0; file.tellg() < end_pos; i++) {
        std::streampos cur_start = file.tellg();
        std::streamoff cur_end = cur_start + static_cast<std::streamoff>(part_size_bytes);

        if (cur_end >= end_pos) {
            file.seekg(0, std::ios::end);
            cur_end = end_pos;
        }
        else {
            std::cerr << end_pos << " " << cur_start << " " << cur_end << " -> ";
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

            std::cerr << cur_end << std::endl;
        }

        std::string split_filepath = filepath + "-split-" + std::to_string(i);
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
}

} // mapreduce