#ifndef NDSS_VECTOR_HPP
#define NDSS_VECTOR_HPP
#include <vector>
#include <iostream>
namespace MDL {
template<typename T>
class Vector : public std::vector<T> {
public:
    Vector(long dims = 0) : std::vector<T>(dims) {}

    size_t dimension() const {
        return this->size();
    }

    double L2() const;

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os,
                                    Vector<U>   & obj);
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
