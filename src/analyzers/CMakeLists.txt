project(meta-analyzers)

add_subdirectory(filters)
add_subdirectory(tokenizers)
add_subdirectory(ngram)
add_subdirectory(tree)

add_library(meta-analyzers analyzer.cpp
                           analyzer_factory.cpp
                           libsvm_analyzer.cpp
                           diff_analyzer.cpp
                           multi_analyzer.cpp)
target_link_libraries(meta-analyzers meta-corpus
                                     meta-filters
                                     meta-ngram-analyzers
                                     meta-tokenizers
                                     meta-tree-analyzers
                                     meta-language-model)
