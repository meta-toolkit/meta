project(meta-succinct)

add_library(meta-succinct compressed_vector.cpp bit_vector.cpp sarray.cpp)
target_link_libraries(meta-succinct meta-io)
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(meta-succinct c++experimental)
endif()
