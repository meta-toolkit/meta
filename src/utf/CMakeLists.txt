project(meta-utf)

add_subdirectory(tools)

add_library(meta-utf segmenter.cpp transformer.cpp utf.cpp)
target_link_libraries(meta-utf PUBLIC meta-definitions)
target_link_libraries(meta-utf PRIVATE ${ICU_LIBRARIES})
target_include_directories(meta-utf PRIVATE SYSTEM ${ICU_INCLUDE_DIRS})

install(TARGETS meta-utf
        EXPORT meta-exports
        DESTINATION lib)
