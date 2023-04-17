project(meta-topics)

add_subdirectory(tools)

add_library(meta-topics inferencer.cpp
                        lda_cvb.cpp
                        lda_cvb_inferencer.cpp
                        lda_gibbs.cpp
                        lda_model.cpp
                        lda_scvb.cpp
                        parallel_lda_gibbs.cpp
                        topic_model.cpp
                        bl_term_scorer.cpp)
target_link_libraries(meta-topics meta-index)

install(TARGETS meta-topics
        EXPORT meta-exports
        DESTINATION lib)
