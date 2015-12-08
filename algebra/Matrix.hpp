#ifndef NDSS_MATRIX_HPP
#define NDSS_MATRIX_HPP
#include "Vector.hpp"

#include <vector>
#include <iostream>
#ifdef USE_EIGEN
#include <eigen3/Eigen/Dense>
#endif
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


    Matrix<T>  dot(const Matrix<T>& oth) const;
    Vector<T>  dot(const Vector<T>& oth) const;
#ifdef USE_EIGEN
    double        maxEigenValue() const;
    Matrix<double>inverse() const;
    Eigen::MatrixXd to_Eigen_matrix_format() const;
    void from_Eigen_matrix(const Eigen::MatrixXd& mat);
#endif
    std::vector<NTL::ZZX> encode(const EncryptedArray &ea) const;

    Matrix& operator*=(const T &val);

    Matrix& operator+=(const Matrix<T> &oth);

    Matrix& operator-=(const Matrix<T> &oth);

    void random(const T &domain);

    Matrix<double> reduce(const double factor) const;

    Matrix<long> div(long factor) const;

    Vector<T> vector() const;

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os,
                                    Matrix<U>   & obj);
    /// @create a submatrix from row r1, to row r2 and from column c1 to column c2.
    Matrix<T> submatrix(long r1, long r2, long c1 = 0, long c2 = 0) const;

    Matrix<T> transpose() const;
};

Matrix<long> eye(long dimension);

template<typename T>
Matrix<T> covariance(const Vector<T> &a, const Vector<T> &b);

} // namespace MDL
#endif // ifndef NDSS_MATRIX_HPP
