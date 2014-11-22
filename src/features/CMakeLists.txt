project(meta-features)

add_subdirectory(tools)

add_library(meta-features feature_selector.cpp)
target_link_libraries(meta-features meta-index ${CMAKE_THREAD_LIBS_INIT})
