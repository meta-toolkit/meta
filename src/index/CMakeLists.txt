project(meta-index)

add_subdirectory(eval)
add_subdirectory(ranker)
add_subdirectory(tools)

add_library(meta-index disk_index.cpp
                       inverted_index.cpp
                       forward_index.cpp
                       string_list.cpp
                       string_list_writer.cpp
                       vocabulary_map.cpp
                       vocabulary_map_writer.cpp)
target_link_libraries(meta-index meta-analyzers
                                 meta-eval
                                 meta-ranker
                                 ${CMAKE_THREAD_LIBS_INIT})
