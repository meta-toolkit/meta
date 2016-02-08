project(meta-io)

add_subdirectory(tools)

set(IO_DEPS meta-util ${ZLIB_LIBRARIES})

if (WIN32)
    add_subdirectory(mman-win32)
    list(APPEND IO_DEPS mman-win32)
endif()

add_library(meta-io filesystem.cpp
                    gzstream.cpp
                    libsvm_parser.cpp
                    mmap_file.cpp)
target_link_libraries(meta-io ${IO_DEPS})
