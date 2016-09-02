project(meta-util)

add_library(meta-util progress.cpp)
target_link_libraries(meta-util meta-definitions ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS meta-util
        EXPORT meta-exports
        DESTINATION lib)
