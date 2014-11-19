project(meta-analyzers)

add_subdirectory(filters)
add_subdirectory(tokenizers)

add_library(meta-analyzers analyzer.cpp
                           analyzer_factory.cpp
                           libsvm_analyzer.cpp
                           multi_analyzer.cpp
                           ngram/ngram_analyzer.cpp
                           ngram/ngram_lex_analyzer.cpp
                           ngram/ngram_pos_analyzer.cpp
                           ngram/ngram_simple_analyzer.cpp
                           ngram/ngram_word_analyzer.cpp
                           tree/branch_analyzer.cpp
                           tree/depth_analyzer.cpp
                           tree/parse_tree.cpp
                           tree/semi_skeleton_analyzer.cpp
                           tree/skeleton_analyzer.cpp
                           tree/subtree_analyzer.cpp
                           tree/tag_analyzer.cpp)
target_link_libraries(meta-analyzers meta-corpus
                                     meta-filters
                                     meta-tokenizers
                                     meta-sequence
                                     meta-crf
                                     meta-sequence-analyzers)
