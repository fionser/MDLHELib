#include "utils/FileUtils.hpp"
#include "algebra/Matrix.hpp"
int main() {
    {
    auto matrix = load_csv("adult.data", 10);
    std::cout << matrix << std::endl;
    }
    {
    auto matrix = load_csv_d("all_float_data");
    auto submatrix = matrix.submatrix(0, matrix.rows() - 1, 0, 4);
    auto sT = submatrix.transpose();
    auto prod = sT.dot(submatrix);
    std::cout << prod << std::endl;
    }
    return 0;
}
