add_executable(diff-test diff_test.cpp)
target_link_libraries(diff-test meta-language-model meta-index)

add_executable(create-dataset create-dataset.cpp)
target_link_libraries(create-dataset meta-language-model meta-index)
