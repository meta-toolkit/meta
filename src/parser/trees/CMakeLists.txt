project(meta-parser-trees)

add_subdirectory(visitors)

add_library(meta-parser-trees node.cpp
                              leaf_node.cpp
                              internal_node.cpp
                              parse_tree.cpp)
target_link_libraries(meta-parser-trees meta-tree-visitors)
