project(meta-corpus)

add_subdirectory(tools)

add_library(meta-corpus corpus.cpp
                        document.cpp
                        file_corpus.cpp
                        line_corpus.cpp)
# some corpus classes use io::parser
target_link_libraries(meta-corpus meta-io)
