project(meta-embeddings)

add_subdirectory(tools)

add_library(meta-embeddings word_embeddings.cpp)
target_link_libraries(meta-embeddings cpptoml meta-util)

install(TARGETS meta-embeddings
        EXPORT meta-exports
        DESTINATION lib)
