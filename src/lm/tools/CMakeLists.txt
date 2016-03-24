add_executable(diff-test diff_test.cpp)
target_link_libraries(diff-test meta-language-model meta-index)

add_executable(sentence-likelihood sentence_likelihood.cpp)
target_link_libraries(sentence-likelihood meta-language-model meta-index)

add_executable(build-mph-lm build_mph_lm.cpp)
target_link_libraries(build-mph-lm meta-language-model meta-io)
