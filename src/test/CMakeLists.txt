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
target_link_libraries(meta-testing meta-index meta-classify meta-parser)

set(UNIT_TEST_EXE unit-test)
include(unit_tests.cmake)
