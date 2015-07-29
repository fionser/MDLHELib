#include "FileUtils.hpp"
#include "fhe/EncryptedArray.h"
#include <fstream>
template<typename T>
struct NumericalParser {
    static T parse(const std::string &a) {}
};

template<>
struct NumericalParser<long> {
    static long parse(const std::string &a)
    {
        return std::strtol(a.c_str(), NULL, 10);
    }
};

template<>
struct NumericalParser<double> {
    static double parse(const std::string &a)
    {
        return std::strtod(a.c_str(), NULL);
    }
};

template<typename T>
MDL::Vector<T>parse_line(const std::string& line)
{
    char delimiter = '\0';

    if (line.find(" ") != std::string::npos) delimiter = ' ';

    if (line.find(",") != std::string::npos) delimiter = ',';

    if (delimiter == '\0') {
        fprintf(stderr, "Warnning: the CSV file maybe invalid\n");
        return MDL::Vector<T>(0);
    }

    MDL::Vector<T> vec;
    size_t pos = 0, next = 0;

    while (next < line.size()) {
        next = line.find(delimiter, pos);
        auto substr = line.substr(pos, next - pos);
        vec.push_back(NumericalParser<T>::parse(substr));
        pos = next + 1;
    }
    return vec;
}

MDL::Matrix<long>load_csv(const std::string& file, long max_lines)
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
        if (line[0] == '#') continue;
        mat.push_back(parse_line<long>(line));
    }
    return mat;
}

MDL::Matrix<double>load_csv_d(const std::string& file, long max_lines)
{
    std::ifstream stream;
    std::string   line;
    bool is_all = false;
    MDL::Matrix<double> mat;

    stream.open(file);

    if (!stream.is_open()) return mat;

    is_all = max_lines <= 0;

    while (!stream.eof() && !stream.bad() && (is_all || max_lines-- > 0)) {
        std::getline(stream, line);

        if (line.empty()) continue;
        if (line[0] == '#') continue;
        mat.push_back(parse_line<double>(line));
    }
    return mat;
}
