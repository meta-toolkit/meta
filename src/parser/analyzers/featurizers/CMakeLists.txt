add_library(meta-parser-featurizers branch_featurizer.cpp
                                    depth_featurizer.cpp
                                    featurizer_factory.cpp
                                    semi_skeleton_featurizer.cpp
                                    skeleton_featurizer.cpp
                                    subtree_featurizer.cpp
                                    tag_featurizer.cpp)

target_link_libraries(meta-parser-featurizers meta-parser-trees
                                              meta-corpus)

install(TARGETS meta-parser-featurizers
        EXPORT meta-exports
        DESTINATION lib)
