project(meta-parser)

add_subdirectory(trees)
add_subdirectory(io)
add_subdirectory(tools)

add_library(meta-parser sr_parser.cpp transition.cpp)
target_link_libraries(meta-parser meta-parser-trees meta-parser-io)
