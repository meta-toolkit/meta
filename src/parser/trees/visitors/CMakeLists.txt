project(meta-tree-visitors)

add_library(meta-tree-visitors annotation_remover.cpp
                               binarizer.cpp
                               debinarizer.cpp
                               empty_remover.cpp
                               head_finder.cpp
                               leaf_node_finder.cpp
                               sequence_extractor.cpp
                               transition_finder.cpp
                               unary_chain_remover.cpp)
