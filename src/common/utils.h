#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace mapreduce {

const std::string INTERMEDIATE_FILE_SUFFIX = "-intermediate.txt";

/**
 * Splits a file into multiple files of aprroximately a given size. 
 * Idividual lines are not split. The last file may be significantly smaller than the given size.
 * Returns the number of files created.
*/
size_t split_file(std::string const& filepath, size_t part_size_mb);

/**
 * Checks if a filepath has a valid format.
 * For now, this means that the filepath ends with ".txt".
*/
bool has_valid_format(std::string const& filepath);

/**
 * Returns the filepath of the intermediate file for a given filepath.
*/
std::string get_intermediate_filepath(std::string const& filepath);

/**
 * Returns the filepath of the split file for a given filepath and split index.
*/
std::string get_split_filepath(std::string const& filepath, size_t split_index);

} // mapreduce

#endif // UTILS_H