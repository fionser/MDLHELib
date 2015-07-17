include_directories(../)
include_directories(../HElib)

add_executable(test_EncryptVector test_encvector.cpp)
add_executable(test_Matrix test_MatrixVector.cpp)
add_executable(test_dump test_dumpFHESetting.cpp)
add_executable(test_fileutils test_fileutils.cpp)
add_executable(test_GT test_GT.cpp)

add_executable(benchmark_FHE_primitives benchmark_FHE_primitives.cpp)
add_executable(benchmark_mean_variance benchmark_mean_variance.cpp)
add_executable(benchmark_PCA benchmark_PCA.cpp)
add_executable(benchmark_percentile benchmark_percentile.cpp)

target_link_libraries(test_EncryptVector algebra utils fhe)
target_link_libraries(test_Matrix algebra utils)
target_link_libraries(test_dump utils fhe)
target_link_libraries(test_fileutils utils fhe)
target_link_libraries(test_GT protocol algebra utils fhe)

target_link_libraries(benchmark_PCA algebra utils fhe)
target_link_libraries(benchmark_FHE_primitives algebra utils fhe)
target_link_libraries(benchmark_mean_variance algebra utils fhe)
target_link_libraries(benchmark_percentile algebra protocol utils fhe)

file(COPY adult_result adult.data DESTINATION .)
