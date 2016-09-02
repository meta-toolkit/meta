project(meta-topics)

add_subdirectory(tools)

add_library(meta-topics lda_cvb.cpp
                        lda_gibbs.cpp
                        lda_model.cpp
                        lda_scvb.cpp
                        parallel_lda_gibbs.cpp)
target_link_libraries(meta-topics meta-index)

install(TARGETS meta-topics
        EXPORT meta-exports
        DESTINATION lib)
