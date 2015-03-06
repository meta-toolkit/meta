project(meta-sequence-analyzers)

add_library(meta-sequence-analyzers ngram_pos_analyzer.cpp)
target_link_libraries(meta-sequence-analyzers meta-analyzers meta-crf)
