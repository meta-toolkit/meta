project(meta-analyzers)

add_subdirectory(filters)
add_subdirectory(tokenizers)
add_subdirectory(tools)

add_library(meta-analyzers analyzer.cpp
                           analyzer_factory.cpp
                           libsvm_analyzer.cpp
                           multi_analyzer.cpp
                           ngram/ngram_analyzer.cpp
                           ngram/ngram_lex_analyzer.cpp
                           ngram/ngram_pos_analyzer.cpp
                           ngram/ngram_simple_analyzer.cpp
                           ngram/ngram_word_analyzer.cpp
                           tree/tree_analyzer.cpp
                           tree/featurizers/branch_featurizer.cpp
                           tree/featurizers/depth_featurizer.cpp
                           tree/featurizers/semi_skeleton_featurizer.cpp
                           tree/featurizers/skeleton_featurizer.cpp
                           tree/featurizers/subtree_featurizer.cpp
                           tree/featurizers/tag_featurizer.cpp
                           tree/featurizers/featurizer_factory.cpp)
target_link_libraries(meta-analyzers meta-corpus
                                     meta-filters
                                     meta-tokenizers
                                     meta-sequence
                                     meta-crf
                                     meta-greedy-tagger
                                     meta-parser
                                     meta-sequence-analyzers)
