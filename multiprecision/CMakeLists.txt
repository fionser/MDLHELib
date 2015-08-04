include_directories(../)
include_directories(../HElib/)
set(LIB_SRCS MPSecKey.cpp MPPubKey.cpp MPContext.cpp MPEncArray.cpp)
add_library(multiprecision STATIC ${LIB_SRCS})
