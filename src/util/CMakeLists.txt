project(meta-util)

add_subdirectory(tools)

if (ZLIB_FOUND)
    add_library(meta-util progress.cpp gzstream.cpp)
    target_link_libraries(meta-util ${ZLIB_LIBRARIES})
else()
    add_library(meta-util progress.cpp)
endif()
