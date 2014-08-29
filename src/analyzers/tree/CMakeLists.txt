project(meta-tree-analyzers)

add_library(meta-tree-analyzers branch_analyzer.cpp
                                depth_analyzer.cpp
                                parse_tree.cpp
                                semi_skeleton_analyzer.cpp
                                skeleton_analyzer.cpp
                                subtree_analyzer.cpp
                                tag_analyzer.cpp)
