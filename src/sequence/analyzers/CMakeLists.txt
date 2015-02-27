project(meta-sequence-analyzers)

add_library(meta-sequence-analyzers sequence_analyzer.cpp)
target_link_libraries(meta-sequence-analyzers meta-util meta-utf meta-io)
