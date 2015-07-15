#include "FileUtils.hpp"

#include <fstream>
MDL::Vector<long>parse_line(const std::string& line)
{
    char delimiter = '\0';
    
    if (line.find(" ") != std::string::npos) delimiter = ' ';

    if (line.find(",") != std::string::npos) delimiter = ',';

    if (delimiter == '\0') {
        fprintf(stderr, "Warnning: the CSV file maybe invalid\n");
        return MDL::Vector<long>(0);
    }

    MDL::Vector<long> vec;
    size_t pos = 0, next = 0;

    while (next < line.size()) {
        next = line.find(delimiter, pos);
        auto substr = line.substr(pos, next - pos);
        vec.push_back(std::strtol(substr.c_str(), NULL, 10));
        pos = next + 1;
    }
    return vec;
}

MDL::Matrix<long>load_csv(const std::string& file, int max_lines)
{
    std::ifstream stream;
    std::string   line;
    bool is_all = false;
    MDL::Matrix<long> mat;

    stream.open(file);

    if (!stream.is_open()) return mat;

    is_all = max_lines <= 0;

    while (!stream.eof() && !stream.bad() && (is_all || max_lines-- > 0)) {
        std::getline(stream, line);

        if (line.empty()) continue;
        mat.push_back(parse_line(line));
    }
    return mat;
}
