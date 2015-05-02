project(meta-io)

add_subdirectory(tools)

set(IO_SOURCES compressed_file_reader.cpp
               compressed_file_writer.cpp
               libsvm_parser.cpp
               mmap_file.cpp
               parser.cpp)

set(IO_DEPS meta-util)

if (ZLIB_FOUND)
    list(APPEND IO_SOURCES gzstream.cpp)
    list(APPEND IO_DEPS ${ZLIB_LIBRARIES})
endif()

if (WIN32)
    add_subdirectory(mman-win32)
    list(APPEND IO_DEPS mman-win32)
endif()

add_library(meta-io ${IO_SOURCES})
target_link_libraries(meta-io ${IO_DEPS})
