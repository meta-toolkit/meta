add_executable(greedy-tagger-train greedy_tagger_train.cpp)
target_link_libraries(greedy-tagger-train meta-greedy-tagger meta-classify)

add_executable(greedy-tagger-test greedy_tagger_test.cpp)
target_link_libraries(greedy-tagger-test meta-greedy-tagger meta-classify)
