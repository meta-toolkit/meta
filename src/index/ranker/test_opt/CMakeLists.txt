project(meta-dirichlet-test)

include_directories(../../../../include)

add_executable(test_opt test.cpp)

target_link_libraries(test_opt meta-ranker)

