include_directories(../)
include_directories(../HElib/)
set(LIB_FILES FHEUtils.cpp FileUtils.cpp GreaterThanUtils.cpp encoding.cpp)
add_library(utils STATIC ${LIB_FILES})
