project(meta-embeddings)

add_subdirectory(tools)
add_subdirectory(analyzers)

add_library(meta-embeddings cooccurrence_counter.cpp word_embeddings.cpp)
target_link_libraries(meta-embeddings cpptoml meta-analyzers meta-util meta-io)

install(TARGETS meta-embeddings
        EXPORT meta-exports
        DESTINATION lib)
