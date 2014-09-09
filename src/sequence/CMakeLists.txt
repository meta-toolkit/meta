project(meta-sequence)

add_subdirectory(analyzers)
add_subdirectory(crf)
add_subdirectory(io)

add_library(meta-sequence observation.cpp
                          sequence.cpp
                          trellis.cpp)
target_link_libraries(meta-sequence meta-sequence-analyzers meta-sequence-io)
