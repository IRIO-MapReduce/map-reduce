#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace mapreduce {

using key_t = std::string;
using val_t = std::string;

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
 * Checks if a filepath exists and is executable.
*/
bool is_executable(std::string const& filepath);

/**
 * Checks if a filepath exists and is executable.
 * Throws an exception if the filepath is not executable.
*/
void validate_executable(std::string const& filepath);

/**
 * Returns the filepath of the intermediate file for a given filepath and index.
*/
std::string get_intermediate_filepath(std::string const& filepath, size_t index);

/**
 * Returns the filepath of the split file for a given filepath and index.
*/
std::string get_split_filepath(std::string const& filepath, size_t index);

} // mapreduce

#endif // UTILS_H