include_directories(../)
include_directories(../HElib/)
set(LIB_FILES FHEUtils.cpp)
add_library(utils STATIC ${LIB_FILES})

