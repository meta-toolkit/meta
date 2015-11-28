project(meta-corpus)

add_subdirectory(tools)

set(CORPUS_SOURCES corpus.cpp
                   corpus_factory.cpp
                   document.cpp
                   file_corpus.cpp
                   libsvm_corpus.cpp
                   line_corpus.cpp
                   metadata.cpp
                   metadata_parser.cpp)

if (ZLIB_FOUND)
    list(APPEND CORPUS_SOURCES gz_corpus.cpp)
endif()

add_library(meta-corpus ${CORPUS_SOURCES})

# some corpus classes use io::parser
target_link_libraries(meta-corpus meta-io meta-utf)
