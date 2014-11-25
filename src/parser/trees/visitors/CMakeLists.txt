project(meta-tree-visitors)

add_library(meta-tree-visitors annotation_remover.cpp
                               empty_remover.cpp
                               unary_chain_remover.cpp)
