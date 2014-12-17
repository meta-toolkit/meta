add_executable(read-trees read_trees.cpp)
target_link_libraries(read-trees meta-parser-io)

add_executable(parser-train parser_train.cpp)
target_link_libraries(parser-train meta-parser meta-util)
