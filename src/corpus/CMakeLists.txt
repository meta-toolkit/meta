project(meta-corpus)

add_subdirectory(tools)

add_library(meta-corpus corpus.cpp
                        corpus_factory.cpp
                        document.cpp
                        file_corpus.cpp
                        libsvm_corpus.cpp
                        line_corpus.cpp
                        gz_corpus.cpp
                        metadata.cpp
                        metadata_parser.cpp)

target_link_libraries(meta-corpus meta-io meta-utf cpptoml)

install(TARGETS meta-corpus
        EXPORT meta-exports
        DESTINATION lib)
