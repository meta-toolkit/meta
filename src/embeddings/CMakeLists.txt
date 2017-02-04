project(meta-embeddings)

add_subdirectory(tools)
add_subdirectory(analyzers)

add_library(meta-embeddings word_embeddings.cpp)
target_link_libraries(meta-embeddings meta-io cpptoml meta-util)

install(TARGETS meta-embeddings
        EXPORT meta-exports
        DESTINATION lib)
