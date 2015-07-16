#include "Matrix.hpp"
#include <eigen3/Eigen/Eigenvalues>
namespace MDL {
template<typename T>
size_t Matrix<T>::cols() const {
    if (rows() == 0) return 0;

    return this->at(0).size();
}

template<>
size_t Matrix<long>::rows() const {
    return this->size();
}

template<>
size_t Matrix<double>::rows() const {
    return this->size();
}

template<typename U>
Eigen::MatrixXd Matrix<U>::to_Eigen_matrix_format() const
{
    Eigen::MatrixXd mat = Eigen::MatrixXd::Random(rows(), cols());

    for (size_t r = 0; r < rows(); r++) {
        for (size_t c = 0; c < cols(); c++) {
            mat(r, c) = static_cast<double>(this->at(r).at(c));
        }
    }
    return mat;
}

template<typename T>
void Matrix<T>::from_Eigen_matrix(const Eigen::MatrixXd& mat)
{
    auto rows = mat.rows();
    auto cols = mat.cols();

    if (rows != this->rows()) {
        this->resize(rows, Vector<T>(cols));
    }

    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            this->at(r).at(c) = static_cast<T>(mat(r, c));
        }
    }
}

template<typename U>
double Matrix<U>::maxEigenValue() const {
    Eigen::MatrixXd eig = to_Eigen_matrix_format();
    Eigen::EigenSolver<Eigen::MatrixXd> eigenSolver(eig);

    return eig.eigenvalues()(0).real();
}

template<typename U>
Matrix<double>Matrix<U>::inverse() const {
    Matrix<double> ret;

    if (rows() != cols()) return ret;

    Eigen::MatrixXd mat = to_Eigen_matrix_format();
    auto inverse = mat.inverse();

    ret.from_Eigen_matrix(inverse);
    return ret;
}

template<>
Matrix<double>Matrix<double>::dot(const Matrix<double>& oth) const {
    auto mat1 = to_Eigen_matrix_format();
    auto mat2 = oth.to_Eigen_matrix_format();

    Matrix<double> prod;
    prod.from_Eigen_matrix(mat1 * mat2);
    return prod;
}

template class Matrix<long>;
template class Matrix<double>;
} // namespace MDL
