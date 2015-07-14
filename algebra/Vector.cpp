#include "Vector.hpp"
#include <NTL/ZZ.h>
#include <NTL/ZZX.h>
#include <cmath>
namespace MDL {
template<>
double Vector<long>::L2() const {
    double norm = 0.0;

    for (auto e : *this) {
        auto de = static_cast<double>(e);
        norm += de * de;
    }
    return std::sqrt(norm);
}

template<>
double Vector<NTL::ZZX>::L2() const {
    NTL::ZZ summation(0);
    for (auto &e : *this) {
        summation += e[0] * e[0];
    }
    return std::sqrt(std::exp(log(summation)));
}

template class Vector<long>;
template class Vector<double>;
template class Vector<NTL::ZZX>;
} // namespace MDL
