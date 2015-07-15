#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP
#include "algebra/Matrix.hpp"

#include <string>
/// @brief load a csv into a matrix
/// @param file. Path of the csv file
/// @param max_lines. The maximum lines to read. if max_lines <= 1 to read all the lines
MDL::Matrix<long>load_csv(const std::string& file, long max_lines = 0);
#endif // FILE_UTILS_HPP
