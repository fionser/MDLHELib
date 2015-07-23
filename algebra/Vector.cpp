#include "Vector.hpp"
#include <NTL/ZZ.h>
#include <NTL/ZZX.h>
#include "fhe/EncryptedArray.h"
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
Vector<double> Vector<T>::reduce(double factor) const {
    Vector<double> vec(dimension());
    std::transform(this->begin(), this->end(), vec.begin(),
                   [&factor](const T &v) { return v / factor; });
    return vec;
}

template<>
Vector<double> Vector<NTL::ZZX>::reduce(double factor) const {
    Vector<double> vec(dimension());
    std::transform(this->begin(), this->end(), vec.begin(),
                   [&factor](const NTL::ZZX &v) {
                   return NTL::to_long(v[0]) / factor;
                   });
    return vec;
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

template<>
NTL::ZZX Vector<long>::encode(const EncryptedArray &ea) const
{
    assert(this->size() <= ea.size());
    NTL::ZZX encoded;
    if (this->size() < ea.size()) {
        auto tmp(*this);
        tmp.resize(ea.size());
        ea.encode(encoded, tmp);
    } else {
        ea.encode(encoded, *this);
    }
    return encoded;
}

    template<typename T>
Vector<T>& Vector<T>::operator*=(const T& val)
{
    for (auto &ele : *this) {
        ele *= val;
    }
    return *this;
}

    template<>
void Vector<long>::random(const long &domain)
{
    for (size_t i = 0; i < size(); i++) {
        this->at(i) = NTL::RandomBnd(domain);
    }
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
