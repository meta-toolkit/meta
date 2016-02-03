add_executable(embedding-vocab embedding_vocab.cpp)
target_link_libraries(embedding-vocab meta-analyzers meta-util meta-io)

add_executable(embedding-coocur embedding_coocur.cpp)
target_link_libraries(embedding-coocur meta-analyzers meta-util meta-io)
