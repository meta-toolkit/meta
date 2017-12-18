project(meta-learn)

add_subdirectory(loss)
add_subdirectory(tools)

add_library(meta-learn sgd.cpp ranker/let.cpp ../../include/meta/learn/ranker/let.h)
target_link_libraries(meta-learn meta-io meta-loss cpptoml)

install(TARGETS meta-learn
        EXPORT meta-exports
        DESTINATION lib)
