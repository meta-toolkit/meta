project(meta-parser-analyzers)

add_subdirectory(featurizers)

add_library(meta-parser-analyzers tree_analyzer.cpp)
target_link_libraries(meta-parser-analyzers meta-analyzers
                                            meta-parser-featurizers
                                            meta-parser
                                            meta-greedy-tagger)

install(TARGETS meta-parser-analyzers
        EXPORT meta-exports
        DESTINATION lib)
