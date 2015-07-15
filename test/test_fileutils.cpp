#include "utils/FileUtils.hpp"

int main() {
    auto matrix = load_csv("adult.data", 10);
    std::cout << matrix << std::endl;
    return 0;
}
