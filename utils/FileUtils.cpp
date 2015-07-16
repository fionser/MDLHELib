#include "FileUtils.hpp"
#include "fhe/EncryptedArray.h"
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
        mat.push_back(parse_line(line));
    }
    return mat;
}

void totalSums(const EncryptedArray& ea, const long r, Ctxt& ctxt)
{
    if (r > ea.size() || r == 1) return;

    Ctxt orig = ctxt;

    long k    = NumBits(r);
    long e    = 1;

    for (long i = k - 2; i >= 0; i--) {
        Ctxt tmp1 = ctxt;
        ea.rotate(tmp1, e);
        ctxt += tmp1; // ctxt = ctxt + (ctxt >>> e)
        e     = 2 * e;

        if (bit(r, i)) {
            Ctxt tmp2 = orig;
            ea.rotate(tmp2, e);
            ctxt += tmp2; // ctxt = ctxt + (orig >>> e)
                          // NOTE: we could have also computed
                          // ctxt =  (ctxt >>> e) + orig, however,
                          // this would give us greater depth/noise
            e += 1;
        }
    }
}
