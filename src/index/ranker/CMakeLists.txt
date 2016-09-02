project(meta-ranker)

add_library(meta-ranker absolute_discount.cpp
                        dirichlet_prior.cpp
                        jelinek_mercer.cpp
                        lm_ranker.cpp
                        okapi_bm25.cpp
                        pivoted_length.cpp
                        ranker.cpp
                        ranker_factory.cpp)
target_link_libraries(meta-ranker meta-index)

install(TARGETS meta-ranker
        EXPORT meta-exports
        DESTINATION lib)
