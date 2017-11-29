project(meta-ranker)

add_library(meta-ranker absolute_discount.cpp
                        dirichlet_prior.cpp
                        jelinek_mercer.cpp
                        lm_ranker.cpp
                        okapi_bm25.cpp
                        pivoted_length.cpp
                        kl_divergence_prf.cpp
                        rocchio.cpp
                        ranker.cpp
                        ranker_factory.cpp wmd_base.cpp)
target_link_libraries(meta-ranker meta-index meta-embeddings meta-util)

install(TARGETS meta-ranker
        EXPORT meta-exports
        DESTINATION lib)
