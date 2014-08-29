project(meta-io)

add_library(meta-io compressed_file_reader.cpp
                    compressed_file_writer.cpp
                    libsvm_parser.cpp
                    mmap_file.cpp
                    parser.cpp)
target_link_libraries(meta-io meta-util)
