project(meta-parser-trees)

add_library(meta-parser-trees node.cpp
                              leaf_node.cpp
                              internal_node.cpp
                              parse_tree.cpp)
