project(meta-io)

add_subdirectory(tools)

set(META_IO_SOURCES filesystem.cpp
                    gzstream.cpp
                    libsvm_parser.cpp
                    mmap_file.cpp)

if (META_HAS_LIBLZMA)
    list(APPEND META_IO_SOURCES xzstream.cpp)
endif()

add_library(meta-io ${META_IO_SOURCES})
target_link_libraries(meta-io meta-util ${ZLIB_LIBRARIES})

if (WIN32)
    add_subdirectory(mman-win32)
    target_link_libraries(meta-io meta-mman-win32)
endif()

if (META_HAS_EXPERIMENTAL_FILESYSTEM)
    target_link_libraries(meta-io ${STD_EXPERIMENTAL_FILESYSTEM_LIBRARIES})
else()
    target_include_directories(meta-io SYSTEM PRIVATE
        ${META_PROJECT_SOURCE_DIR}/deps/meta-stlsoft/include)
endif()

if (META_HAS_LIBLZMA)
    target_include_directories(meta-io PUBLIC ${LIBLZMA_INCLUDE_DIRS})
    target_link_libraries(meta-io ${LIBLZMA_LIBRARIES})
endif()

install(TARGETS meta-io
        EXPORT meta-exports
        DESTINATION lib)
