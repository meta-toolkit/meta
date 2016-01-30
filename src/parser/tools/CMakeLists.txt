add_executable(read-trees read_trees.cpp)
target_link_libraries(read-trees meta-parser)

add_executable(sr-parse sr_parse.cpp)
target_link_libraries(sr-parse meta-parser meta-analyzers meta-greedy-tagger)

add_executable(parser-train parser_train.cpp)
target_link_libraries(parser-train meta-parser meta-util cpptoml)

add_executable(parser-test parser_test.cpp)
target_link_libraries(parser-test meta-parser meta-util cpptoml)
