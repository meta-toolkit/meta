add_executable(lda lda.cpp)
target_link_libraries(lda meta-topics)

add_executable(lda-topics lda-topics.cpp)
target_link_libraries(lda-topics meta-index)
