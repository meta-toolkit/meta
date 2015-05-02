project(meta-parser-trees)

add_subdirectory(visitors)

add_library(meta-parser-trees leaf_node.cpp
                              node.cpp
                              internal_node.cpp
                              parse_tree.cpp)
