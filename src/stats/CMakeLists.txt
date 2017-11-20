project(meta-stats)

add_library(meta-stats running_stats.cpp optimization.cpp)
target_link_libraries(meta-stats meta-definitions)

install(TARGETS meta-stats
        EXPORT meta-exports
        DESTINATION lib)
