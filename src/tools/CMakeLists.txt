add_executable(profile profile.cpp)
target_link_libraries(profile meta-index
                              meta-greedy-tagger
                              meta-parser
                              ${CMAKE_THREAD_LIBS_INIT})

add_executable(top-k top_k.cpp)
target_link_libraries(top-k meta-index)

add_executable(mph-vocab mph_vocab.cpp)
target_link_libraries(mph-vocab meta-io meta-util meta-succinct)
