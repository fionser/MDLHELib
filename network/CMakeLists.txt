include_directories(../)
set(LIB_SRCS network.cpp)
add_library(net STATIC ${LIB_SRCS})
