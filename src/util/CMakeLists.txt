project(meta-util)

add_library(meta-util progress.cpp)
target_link_libraries(meta-util meta-definitions)
