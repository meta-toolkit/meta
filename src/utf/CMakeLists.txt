project(meta-utf)

add_subdirectory(tools)

add_library(meta-utf segmenter.cpp transformer.cpp utf.cpp)
target_link_libraries(meta-utf ${ICU_LIBRARIES})
