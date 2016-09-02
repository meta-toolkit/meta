project(meta-analyzers)

add_subdirectory(filters)
add_subdirectory(tokenizers)
add_subdirectory(tools)

add_library(meta-analyzers analyzer.cpp
                           analyzer_factory.cpp
                           multi_analyzer.cpp
                           ngram/ngram_analyzer.cpp
                           ngram/ngram_word_analyzer.cpp)
target_link_libraries(meta-analyzers meta-corpus
                                     meta-filters
                                     meta-tokenizers)

install(TARGETS meta-analyzers
        EXPORT meta-exports
        DESTINATION lib)
