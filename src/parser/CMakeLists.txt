project(meta-parser)

add_subdirectory(analyzers)
add_subdirectory(io)
add_subdirectory(tools)
add_subdirectory(trees)

add_library(meta-parser sr_parser.cpp
                        state.cpp
                        state_analyzer.cpp
                        sequence_extractor.cpp
                        training_data.cpp
                        transition.cpp
                        transition_finder.cpp
                        transition_map.cpp
                        evalb.cpp)
target_link_libraries(meta-parser meta-parser-trees
                                  meta-tree-visitors
                                  meta-parser-io
                                  meta-io
                                  meta-sequence
                                  meta-util
                                  ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS meta-parser
        EXPORT meta-exports
        DESTINATION lib)
