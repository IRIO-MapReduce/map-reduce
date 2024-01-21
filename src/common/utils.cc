#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

namespace mapreduce {

size_t split_file_bytes(std::string const& filepath, size_t part_size_bytes)
{
    // 1 KB overhead for splitting lines
    static constexpr size_t MAX_LINE_SIZE_BYTES = 1024;

    if (part_size_bytes == 0) {
        throw std::invalid_argument("part_size_mb must be greater than 0");
    }

    std::ifstream file(filepath, std::ios::ate);
    if (!file.is_open()) {
        throw std::invalid_argument("could not open file");
    }

    char* buffer = new char[part_size_bytes + MAX_LINE_SIZE_BYTES];

    std::streampos end_pos = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t num_parts;
    for (num_parts = 0; file.tellg() < end_pos; num_parts++) {
        std::streampos cur_start = file.tellg();
        std::streamoff cur_end
            = cur_start + static_cast<std::streamoff>(part_size_bytes);

        if (cur_end >= end_pos) {
            file.seekg(0, std::ios::end);
            cur_end = end_pos;
        } else {
            std::string temp;
            file.seekg(cur_end);

            if (!getline(file, temp)) {
                delete[] buffer;
                throw std::runtime_error("error while cropping lines");
            }

            if (file.eof()) {
                cur_end = end_pos;
            } else {
                cur_end = file.tellg();
            }
        }

        std::string split_filepath = filepath;
        split_filepath = combine_filepath(filepath, num_parts);
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
        } else {
            file.read(buffer, num_bytes);
            split_file.write(buffer, num_bytes);
        }
    }

    delete[] buffer;
    return num_parts;
}

bool has_valid_format(std::string const& filepath)
{
    return std::filesystem::exists(filepath) && filepath.length() >= 4
        && filepath.substr(filepath.length() - 4) == ".txt";
}

bool is_executable(std::string const& filepath)
{
    if (!std::filesystem::exists(filepath)) {
        return false;
    }
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IXUSR) != 0;
}

void validate_executable(std::string const& filepath)
{
    if (!is_executable(filepath)) {
        throw std::invalid_argument("invalid executable filepath");
    }
}

std::string combine_filepath(std::string const& filepath, std::string str)
{
    std::string combined_filepath = filepath;
    combined_filepath.erase(combined_filepath.length() - 4);
    combined_filepath += "-" + str + ".txt";
    return combined_filepath;
}

std::string combine_filepath(std::string const& filepath, uint32_t idx)
{
    return combine_filepath(filepath, std::to_string(idx));
}

std::string get_random_string()
{
    static constexpr char ALPHABET[]
        = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string random_string;
    random_string.resize(FILE_HASH_LENGTH, '0');

    for (size_t i = 0; i < FILE_HASH_LENGTH; i++) {
        random_string[i] = ALPHABET[rand() % (sizeof(ALPHABET) - 1)];
    }

    return random_string;
}

std::string hash_filepath(std::string const& filepath, std::string const& hash)
{
    std::string hashed_filepath = filepath;
    hashed_filepath.erase(hashed_filepath.length() - 4);
    hashed_filepath += "-" + hash + ".txt";
    return hashed_filepath;
}

std::string unhash_filepath(std::string const& filepath)
{
    std::string unhashed_filepath = filepath;
    unhashed_filepath.erase(unhashed_filepath.length() - FILE_HASH_LENGTH - 5);
    unhashed_filepath += ".txt";
    return unhashed_filepath;
}

} // mapreduce