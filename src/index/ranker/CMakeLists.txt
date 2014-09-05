project(meta-ranker)

add_library(meta-ranker absolute_discount.cpp
                        dirichlet_prior.cpp
                        jelinek_mercer.cpp
                        lm_ranker.cpp
                        okapi_bm25.cpp
                        pivoted_length.cpp
                        ranker.cpp
                        ranker_factory.cpp)
