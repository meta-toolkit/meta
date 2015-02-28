project(meta-corpus)

add_subdirectory(tools)

if (ZLIB_FOUND)
    add_library(meta-corpus corpus.cpp
                            document.cpp
                            file_corpus.cpp
                            line_corpus.cpp
                            gz_corpus.cpp)
else()
    add_library(meta-corpus corpus.cpp
                            document.cpp
                            file_corpus.cpp
                            line_corpus.cpp)
endif()
# some corpus classes use io::parser
target_link_libraries(meta-corpus meta-io)
