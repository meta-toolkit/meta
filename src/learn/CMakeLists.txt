project(meta-learn)

add_subdirectory(loss)
add_subdirectory(tools)

add_library(meta-learn sgd.cpp learntorank/pairwise_letor.cpp ../../include/meta/learn/learntorank/pairwise_letor.h)
target_link_libraries(meta-learn meta-io meta-loss cpptoml)

install(TARGETS meta-learn
        EXPORT meta-exports
        DESTINATION lib)
