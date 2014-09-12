project(meta-crf)

add_subdirectory(tools)

add_library(meta-crf crf.cpp
                     scorer.cpp
                     tagger.cpp
                     viterbi_scorer.cpp)
target_link_libraries(meta-crf meta-sequence)
