project(meta-embeddings-analyzers)

add_library(meta-embeddings-analyzers embedding_analyzer.cpp)
target_link_libraries(meta-embeddings-analyzers meta-analyzers meta-embeddings)

install(TARGETS meta-embeddings-analyzers
        EXPORT meta-exports
        DESTINATION lib)
