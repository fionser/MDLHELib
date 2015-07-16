include_directories(../HElib)
include_directories(../)
set(LIB_SRCS Gt.cpp)
add_library(protocol STATIC ${LIB_SRCS})
