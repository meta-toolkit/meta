project(meta-hmm)

add_subdirectory(tools)

add_library(meta-hmm sequence_observations.cpp)
target_link_libraries(meta-hmm meta-sequence)

install(TARGETS meta-hmm
        EXPORT meta-exports
        DESTINATION lib)
