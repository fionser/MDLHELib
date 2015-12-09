include_directories(../)
include_directories(../HElib/)
set(LIB_SRCS MPSecKey.cpp MPPubKey.cpp MPContext.cpp MPEncArray.cpp
    MPEncVector.cpp MPEncMatrix.cpp MPReplicate.cpp MPRotate.cpp
    MPSwap.cpp MPRepeat.cpp)
add_library(multiprecision STATIC ${LIB_SRCS})
