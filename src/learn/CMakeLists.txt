project(meta-learn)

add_subdirectory(loss)

add_library(meta-learn sgd.cpp)
target_link_libraries(meta-learn meta-io meta-loss cpptoml)

install(TARGETS meta-learn
        EXPORT meta-exports
        DESTINATION lib)
