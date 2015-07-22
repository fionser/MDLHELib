#ifndef NDSS_MATRIX_HPP
#define NDSS_MATRIX_HPP
#include "Vector.hpp"

#include <vector>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <NTL/ZZX.h>
class EncryptedArray;
namespace MDL {
template<typename T>
class Matrix : public std::vector<Vector<T> > {
public:
    Matrix(int rows = 0, int cols = 0)
        : std::vector<Vector<T> >(rows, Vector<T>(cols)) {}

    size_t rows() const;
    size_t cols() const;

    double        maxEigenValue() const;
    Matrix<double>inverse() const;

    Matrix<T>  dot(const Matrix<T>& oth) const;
    Eigen::MatrixXd to_Eigen_matrix_format() const;

    void from_Eigen_matrix(const Eigen::MatrixXd& mat);

    std::vector<NTL::ZZX> encode(const EncryptedArray &ea) const;

    Matrix& operator*=(const T &val);

    void random(const T &domain);

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os,
                                    Matrix<U>   & obj);
};

template<typename U>
std::ostream& operator<<(std::ostream& os, Matrix<U>& obj)
{
    std::cout << "[" << std::endl;

    for (auto& row : obj) {
        std::cout << row << std::endl;
    }
    std::cout << "]";
    return os;
}

Matrix<long> eye(long dimension);
} // namespace MDL
#endif // ifndef NDSS_MATRIX_HPP
