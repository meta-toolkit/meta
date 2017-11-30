project(meta-embeddings)

add_library(meta-wmd wm_distance.cpp)
target_link_libraries(meta-wmd meta-embeddings)

install(TARGETS meta-wmd
        EXPORT meta-exports
        DESTINATION lib)
