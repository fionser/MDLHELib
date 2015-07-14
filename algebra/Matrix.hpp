#ifndef NDSS_MATRIX_HPP
#define NDSS_MATRIX_HPP
#include "Vector.hpp"

#include <vector>
#include <iostream>
#include <eigen3/Eigen/Dense>

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

    Matrix<double>  dot(const Matrix<double>& oth) const;
    Eigen::MatrixXd to_Eigen_matrix_format() const;

    void from_Eigen_matrix(const Eigen::MatrixXd& mat);

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
} // namespace MDL
#endif // ifndef NDSS_MATRIX_HPP
