project(meta-parser-io)

add_library(meta-parser-io ptb_reader.cpp)
target_link_libraries(meta-parser-io meta-parser-trees)

install(TARGETS meta-parser-io
        EXPORT meta-exports
        DESTINATION lib)
