project(meta-io)

add_subdirectory(tools)

set(META_IO_SOURCES filesystem.cpp
                    gzstream.cpp
                    libsvm_parser.cpp
                    mmap_file.cpp)
if (WIN32)
    list(APPEND META_IO_SOURCES mman-win32/mman.c)
endif()

add_library(meta-io ${META_IO_SOURCES})
target_link_libraries(meta-io meta-util ${ZLIB_LIBRARIES})

if (META_HAS_EXPERIMENTAL_FILESYSTEM)
    target_link_libraries(meta-io ${STD_EXPERIMENTAL_FILESYSTEM_LIBRARIES})
else()
    target_include_directories(meta-io SYSTEM PRIVATE
        ${META_PROJECT_SOURCE_DIR}/deps/meta-stlsoft/include)
endif()

install(TARGETS meta-io
        EXPORT meta-exports
        DESTINATION lib)
