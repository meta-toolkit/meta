add_executable(crf-train crf_train.cpp)
target_link_libraries(crf-train meta-crf cpptoml)

add_executable(crf-test crf_test.cpp)
target_link_libraries(crf-test meta-crf meta-classify cpptoml)

add_executable(pos-tag pos_tag.cpp)
target_link_libraries(pos-tag meta-tokenizers meta-crf)

add_executable(pos-tokenizer pos_tokenizer.cpp)
target_link_libraries(pos-tokenizer meta-analyzers meta-crf)
