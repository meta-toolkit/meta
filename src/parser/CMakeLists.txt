project(meta-parser)

add_subdirectory(analyzers)
add_subdirectory(io)
add_subdirectory(tools)
add_subdirectory(trees)

add_library(meta-parser sr_parser.cpp
                        state.cpp
                        state_analyzer.cpp
                        training_data.cpp
                        transition.cpp
                        transition_map.cpp)
target_link_libraries(meta-parser meta-parser-trees
                                  meta-parser-io
                                  meta-io
                                  meta-sequence
                                  meta-util
                                  ${CMAKE_THREAD_LIBS_INIT})
