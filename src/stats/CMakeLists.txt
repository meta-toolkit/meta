project(meta-stats)

add_library(meta-stats running_stats.cpp)
target_link_libraries(meta-stats meta-definitions)

install(TARGETS meta-stats
        EXPORT meta-exports
        DESTINATION lib)
