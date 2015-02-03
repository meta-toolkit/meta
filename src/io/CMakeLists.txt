project(meta-io)

add_subdirectory(tools)

if (ZLIB_FOUND)
    add_library(meta-io compressed_file_reader.cpp
                        compressed_file_writer.cpp
                        gzstream.cpp
                        libsvm_parser.cpp
                        mmap_file.cpp
                        parser.cpp)
    target_link_libraries(meta-io meta-util ${ZLIB_LIBRARIES})
else()
    add_library(meta-io compressed_file_reader.cpp
                        compressed_file_writer.cpp
                        libsvm_parser.cpp
                        mmap_file.cpp
                        parser.cpp)
    target_link_libraries(meta-io meta-util)
endif()
