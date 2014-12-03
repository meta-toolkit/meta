project(meta-tree-visitors)

add_library(meta-tree-visitors annotation_remover.cpp
                               binarizer.cpp
                               empty_remover.cpp
                               head_finder.cpp
                               unary_chain_remover.cpp)
