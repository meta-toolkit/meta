add_executable(embedding-vocab embedding_vocab.cpp)
target_link_libraries(embedding-vocab meta-analyzers meta-util meta-io)
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(embedding-vocab c++experimental)
endif()

add_executable(embedding-coocur embedding_coocur.cpp)
target_link_libraries(embedding-coocur meta-analyzers meta-util meta-io)
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(embedding-coocur c++experimental)
endif()

add_executable(glove glove.cpp)
target_link_libraries(glove meta-util
                            meta-io
                            cpptoml
                            ${CMAKE_THREAD_LIBS_INIT})
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(glove c++experimental)
endif()

add_executable(interactive-embeddings interactive_embeddings.cpp)
target_link_libraries(interactive-embeddings meta-embeddings)
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(interactive-embeddings c++experimental)
endif()

add_executable(meta-to-glove meta_to_glove.cpp)
target_link_libraries(meta-to-glove meta-util
                                    meta-io
                                    cpptoml)
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(meta-to-glove c++experimental)
endif()
