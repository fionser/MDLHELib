#include "Matrix.hpp"
#include "fhe/EncryptedArray.h"
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
    auto values = eig.eigenvalues();
    double maxEig = values(0).real();
    for (size_t i = 1; i < values.size(); i++) {
        maxEig = std::max(maxEig, values(i).real());
    }
    return maxEig;
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

template<>
Matrix<long>Matrix<long>::dot(const Matrix<long>& oth) const {
    assert(this->cols() == oth.rows());
    auto ret(*this);
    auto othT = oth.transpose();
    for (size_t r = 0; r < this->rows(); r++) {
        const auto& row = this->at(r);

        for (size_t c = 0; c < this->cols(); c++) {
            ret[r][c] = row.dot(othT.at(c));
        }
    }
    return ret;
}

template<>
std::vector<NTL::ZZX> Matrix<long>::encode(const EncryptedArray &ea) const
{
    std::vector<NTL::ZZX> encoded_rows;
    for (const auto &row : *this) {
        encoded_rows.push_back(row.encode(ea));
    }
    return encoded_rows;
}

template<typename T>
Matrix<T>& Matrix<T>::operator*=(const T &val)
{
    for (auto &row : *this) {
        row *= val;
    }
    return *this;
}

template<>
void Matrix<long>::random(const long &domain)
{
    for (auto &row : *this) {
        row.random(domain);
    }
}

template<typename T>
Vector<T> Matrix<T>::vector() const
{
    Vector<T> vec(rows() * cols());
    auto itr = vec.begin();

    for (auto &row : *this) {
        std::copy(row.begin(), row.end(), itr);
        std::advance(itr, cols());
    }
    return vec;
}

template<>
Matrix<long>covariance(const Vector<long>& a, const Vector<long>& b)
{
    auto dim = a.dimension();
    assert(dim == b.dimension());
    Matrix<long> mat(dim, dim);
    for (auto i = 0; i < dim; i++) {
        mat[i] = a;
        for (auto j = 0; j < dim; j++) mat[i][j] *= b[i];
    }
    return mat;
}

template<typename T>
Matrix<double> Matrix<T>::reduce(const double factor) const
{
    Matrix<double> mat(rows(), cols());

    for (auto r = 0; r < rows(); r++) {
        mat[r] = this->at(r).reduce(factor);
    }
    return mat;
}

template<typename T>
Matrix<T> Matrix<T>::submatrix(long r1, long r2, long c1, long c2) const
{
    c2 = c2 == 0 ? cols() - 1 : c2;
    if (r2 <= r1 || r2 >= this->size() || c2 <= c1 || c2 > this->at(0).size()) {
        std::cerr << "Invalid submatrix arguments" << std::endl;
        return *this;
    }

    Matrix<T> sub(r2 - r1 + 1, 0);
    for (long r = r1; r <= r2; r++) {
        sub[r - r1] = this->at(r).subvector(c1, c2);
    }
    return sub;
}

template<typename T>
Matrix<T> Matrix<T>::transpose() const
{
    Matrix<T> mat(cols(), rows());
    for (size_t r = 0; r < rows(); r++) {
        for (size_t c = 0; c < cols(); c++) {
            mat[c][r] = this->at(r).at(c);
        }
    }
    return mat;
}

template class Matrix<long>;
template class Matrix<double>;

Matrix<long> eye(long dimension)
{
    Matrix<long> I(dimension, dimension);
    for (long d = 0; d < dimension; d++) {
        I[d][d] = 1;
    }
    return I;
}

template<>
std::ostream& operator<<(std::ostream& os, Matrix<long>& obj)
{
    std::cout << "[" << std::endl;

    for (auto& row : obj) {
        std::cout << row << std::endl;
    }
    std::cout << "]";
    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, Matrix<double>& obj)
{
    std::cout << "[" << std::endl;

    for (auto& row : obj) {
        std::cout << row << std::endl;
    }
    std::cout << "]";
    return os;
}
} // namespace MDL
