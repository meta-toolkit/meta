add_executable(lm-test lm-test.cpp)
target_link_libraries(lm-test meta-language-model meta-index)

add_executable(create-dataset create-dataset.cpp)
target_link_libraries(create-dataset meta-language-model meta-index)
