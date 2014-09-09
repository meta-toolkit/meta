project(meta-language-model)

add_subdirectory(tools)

add_library(meta-language-model language_model.cpp)
target_link_libraries(meta-language-model meta-corpus)
