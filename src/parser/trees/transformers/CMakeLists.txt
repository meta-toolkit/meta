project(meta-tree-transformers)

add_library(meta-tree-transformers annotation_remover.cpp
                                   empty_remover.cpp
                                   unary_chain_remover.cpp)
