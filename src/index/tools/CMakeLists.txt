add_executable(query-runner query_runner.cpp)
target_link_libraries(query-runner meta-ranker
                                   meta-sequence-analyzers
                                   meta-parser-analyzers)

add_executable(search search.cpp)
target_link_libraries(search meta-ranker
                             meta-sequence-analyzers
                             meta-parser-analyzers)

add_executable(index index.cpp)
target_link_libraries(index meta-index
                            meta-sequence-analyzers
                            meta-parser-analyzers)

add_executable(interactive-search interactive_search.cpp)
target_link_libraries(interactive-search meta-ranker
                                         meta-sequence-analyzers
                                         meta-parser-analyzers)

add_executable(print-vocab print_vocab.cpp)
target_link_libraries(print-vocab meta-index)

add_executable(search-vocab search_vocab.cpp)
target_link_libraries(search-vocab meta-index)

add_executable(forward-to-libsvm forward_to_libsvm.cpp)
target_link_libraries(forward-to-libsvm meta-index)
