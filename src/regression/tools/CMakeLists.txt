add_executable(regression regression.cpp)
target_link_libraries(regression meta-regression
                                 meta-index
                                 meta-stats
                                 meta-sequence-analyzers
                                 meta-parser-analyzers)
