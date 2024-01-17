#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace mapreduce {

using key_t = std::string;
using val_t = std::string;

/**
 * Id contains a client request id and job id packed in a single 64-bit integer.
*/
using req_id_t = uint64_t;

/**
 * Splits a file into multiple files of aprroximately a given size. 
 * Idividual lines are not split. The last file may be significantly smaller than the given size.
 * Returns the number of files created.
*/
size_t split_file_bytes(std::string const& filepath, size_t part_size_bytes);

/**
 * Checks if a filepath has a valid format.
 * For now, this means that the filepath ends with ".txt".
*/
bool has_valid_format(std::string const& filepath);

/**
 * Returns the filepath of the intermediate file for a given filepath and index.
*/
std::string get_intermediate_filepath(std::string const& filepath, size_t index);

/**
 * Returns the filepath of the split file for a given filepath and index.
*/
std::string get_split_filepath(std::string const& filepath, size_t index);

constexpr inline uint32_t get_client_id(req_id_t id) { return id >> 32; }

constexpr inline uint32_t get_job_id(req_id_t id) { return id & 0xFFFFFFFF; }

constexpr inline std::pair<uint32_t, uint32_t> extract_ids(req_id_t id) { return {get_client_id(id), get_job_id(id)}; }

constexpr inline req_id_t make_req_id(uint32_t client_id, uint32_t job_id) { return (static_cast<req_id_t>(client_id) << 32) | job_id; }

} // mapreduce

#endif // UTILS_H