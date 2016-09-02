project(meta-tokenizers)

add_library(meta-tokenizers character_tokenizer.cpp
                            icu_tokenizer.cpp
                            whitespace_tokenizer.cpp)
target_link_libraries(meta-tokenizers meta-utf cpptoml)

install(TARGETS meta-tokenizers
        EXPORT meta-exports
        DESTINATION lib)
