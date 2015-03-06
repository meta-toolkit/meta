add_executable(query-runner query-runner.cpp)
target_link_libraries(query-runner meta-index
                                   meta-sequence-analyzers
                                   meta-parser-analyzers)

add_executable(search search.cpp)
target_link_libraries(search meta-index
                             meta-sequence-analyzers
                             meta-parser-analyzers)

add_executable(index index.cpp)
target_link_libraries(index meta-index
                            meta-sequence-analyzers
                            meta-parser-analyzers)

add_executable(interactive-search interactive-search.cpp)
target_link_libraries(interactive-search meta-index
                                         meta-sequence-analyzers
                                         meta-parser-analyzers)

add_executable(print-vocab print_vocab.cpp)
target_link_libraries(print-vocab meta-index)

add_executable(search-vocab search-vocab.cpp)
target_link_libraries(search-vocab meta-index)
