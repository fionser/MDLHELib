#include "Vector.hpp"
#include <NTL/ZZ.h>
#include <NTL/ZZX.h>
#include <cmath>
#include <cassert>
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

template<typename T>
void Vector<T>::reduce(long factor) {}

template<>
void Vector<long>::reduce(long factor) {
    for (size_t i = 0; i < dimension(); i++) {
        this->at(i) /= factor;
    }
}

template<>
double Vector<NTL::ZZX>::L2() const {
    NTL::ZZ summation(0);

    for (auto& e : *this) {
        summation += e[0] * e[0];
    }
    return std::sqrt(std::exp(log(summation)));
}

template<typename T>
T Vector<T>::dot(const Vector<T>& oth) const {
    assert(dimension() == oth.dimension());
    T sum(0);

    for (size_t i = 0; i < dimension(); i++) {
        sum += this->at(i) * oth[i];
    }
    return sum;
}

// template<typename T>
// Matrix<T>Vector<T>::covariance(const Vector<T>& oth) const
// {
//     assert(dimension() == oth.dimension());
//     Matrix<T> mat(dimension(), dimension());
//     for (size_t i = 0; i < dimension(); i++) {
//         mat[i] = *this;
//         for (size_t j = 0; j < dimension(); j++) {
//             mat[i][j] *= oth[j];
//         }
//     }
//     return mat;
// }

template class Vector<long>;
template class Vector<double>;
template class Vector<NTL::ZZX>;
} // namespace MDL
