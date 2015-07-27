#ifndef NDSS_VECTOR_HPP
#define NDSS_VECTOR_HPP
#include <vector>
#include <iostream>
#include <NTL/ZZX.h>
class EncryptedArray;
namespace MDL {
template<typename T>
class Vector : public std::vector<T> {
public:
    Vector(long dims = 0) : std::vector<T>(dims) {}

    Vector(long dims, const T &init) : std::vector<T>(dims, init) {}

    Vector& operator=(const std::vector<T> &oth) {
        auto tmp(oth);
        this->swap(tmp);
        return *this;
    }

    T dot(const Vector<T> &oth) const;

    size_t dimension() const {
        return this->size();
    }

    double L2() const;

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os,
                                    Vector<U>   & obj);

    Vector<double> reduce(double factor) const;

    NTL::ZZX encode(const EncryptedArray &ea) const;

    void random(const T &domain);

    Vector& operator*=(const T &val);

    Vector<T> subvector(long startIndex, long endIndex) const;
};

template<typename U>
std::ostream& operator<<(std::ostream& os, Vector<U>& obj)
{
    std::cout << "[";

    for (auto& e : obj) {
        std::cout << " " << e;
    }
    std::cout << " ]";
    return os;
}
} // namespace MDL
#endif // ifndef NDSS_VECTOR_HPP
