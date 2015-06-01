add_executable(classify classify.cpp)
target_link_libraries(classify meta-classify
                               meta-sequence-analyzers
                               meta-parser-analyzers)

add_executable(online-classify online-classify.cpp)
target_link_libraries(online-classify meta-classify
                                      meta-sequence-analyzers
                                      meta-parser-analyzers)
