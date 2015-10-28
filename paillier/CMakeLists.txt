include_directories(../)
set(LIB_SRCS Paillier.cpp)
add_library(paillier STATIC ${LIB_SRCS})
