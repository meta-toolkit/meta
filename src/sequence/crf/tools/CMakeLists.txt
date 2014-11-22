add_executable(crf-train crf-train.cpp)
target_link_libraries(crf-train meta-crf)

add_executable(crf-test crf-test.cpp)
target_link_libraries(crf-test meta-crf meta-classify)

add_executable(pos-tag pos_tag.cpp)
target_link_libraries(pos-tag meta-tokenizers meta-crf)
