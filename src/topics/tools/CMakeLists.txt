add_executable(lda lda.cpp)
target_link_libraries(lda meta-topics)

add_executable(lda-topics lda_topics.cpp)
target_link_libraries(lda-topics meta-index)

add_executable(topic-corpus topic_corpus.cpp)
target_link_libraries(topic-corpus meta-topics)
