project(meta-parser-trees)

add_library(meta-parser-trees leaf_node.cpp
                              node.cpp
                              internal_node.cpp
                              parse_tree.cpp)
target_link_libraries(meta-parser-trees meta-definitions)

add_subdirectory(visitors)
