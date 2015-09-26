project(meta-learn)

add_subdirectory(loss)

add_library(meta-learn sgd.cpp)
target_link_libraries(meta-learn meta-io meta-loss)
