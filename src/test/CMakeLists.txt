add_subdirectory(tools)

add_library(meta-testing analyzer_test.cpp
                         classifier_test.cpp
                         compression_test.cpp
                         filter_test.cpp
                         forward_index_test.cpp
                         inverted_index_test.cpp
                         ir_eval_test.cpp
                         libsvm_parser_test.cpp
                         parallel_test.cpp
                         ranker_test.cpp
                         stemmer_test.cpp
                         string_list_test.cpp
                         graph_test.cpp
                         vocabulary_map_test.cpp
                         parser_test.cpp)
target_link_libraries(meta-testing meta-index meta-classify meta-parser-io)

add_test(analyzers unit-test analyzers)
set_tests_properties(analyzers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(stemmers unit-test stemmers)
set_tests_properties(stemmers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(parallel unit-test parallel)
set_tests_properties(parallel PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(inverted-index unit-test inverted-index)
set_tests_properties(inverted-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(forward-index unit-test forward-index)
set_tests_properties(forward-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(string-list unit-test string-list)
set_tests_properties(string-list PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(vocabulary-map unit-test vocabulary-map)
set_tests_properties(vocabulary-map PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(libsvm-parser unit-test libsvm-parser)
set_tests_properties(libsvm-parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(classifiers unit-test classifiers)
set_tests_properties(classifiers PROPERTIES TIMEOUT 80 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(rankers unit-test rankers)
set_tests_properties(rankers PROPERTIES TIMEOUT 75 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(ir-eval unit-test ir-eval)
set_tests_properties(ir-eval PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(compression unit-test compression)
set_tests_properties(compression PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(graph unit-test graph)
set_tests_properties(graph PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(parser unit-test parser)
set_tests_properties(parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
