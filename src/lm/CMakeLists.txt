project(meta-language-model)

add_subdirectory(tools)
add_subdirectory(analyzers)

add_library(meta-language-model language_model.cpp
                                token_list.cpp
                                diff.cpp
                                mph_language_model.cpp
                                static_probe_map.cpp
                                sentence.cpp)
target_link_libraries(meta-language-model meta-corpus
                                          meta-analyzers
                                          meta-succinct)

install(TARGETS meta-language-model
        EXPORT meta-exports
        DESTINATION lib)
