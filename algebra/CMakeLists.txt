include_directories(../)
include_directories(../HElib/)
set(LIB_SRCS Matrix.cpp Vector.cpp EncVector.cpp EncMatrix.cpp)
add_library(algebra STATIC ${LIB_SRCS})
