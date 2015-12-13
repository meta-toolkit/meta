ExternalProject_Add(ceeaus
  SOURCE_DIR ${meta_BINARY_DIR}/../../data/ceeaus
  DOWNLOAD_DIR ${meta_BINARY_DIR}/downloads
  URL https://meta-toolkit.org/data/ceeaus.tar.gz
  URL_HASH "SHA256=8ea40b32f34e9ae8aedffe562ad468fc465d1cc0ff6a5c3bdf0ee42bb85c231e"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

ExternalProject_Add(breast-cancer
  SOURCE_DIR ${meta_BINARY_DIR}/../../data/breast-cancer
  DOWNLOAD_DIR ${meta_BINARY_DIR}/../downloads
  URL https://meta-toolkit.org/data/breast-cancer.tar.gz
  URL_HASH "SHA256=28a87b30d651c9b14ebf4e125c9f532233d7970b7bb19e4b1cb8c4f4ccd0d125"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

# Ignore sign warnings when expanding bandit's macros.
file(GLOB IGNORE_BANDIT_WARNING_FILES *.cpp)
set_property(SOURCE ${IGNORE_BANDIT_WARNING_FILES} APPEND PROPERTY COMPILE_FLAGS
    " -Wno-sign-conversion -Wno-conversion")
add_executable(unit-test unit_test.cpp
                         analyzer_test.cpp
                         stemmer_test.cpp
                         parallel_test.cpp
                         inverted_index_test.cpp
                         forward_index_test.cpp
                         classifier_test.cpp
                         string_list_test.cpp
                         vocabulary_map_test.cpp
                         libsvm_parser_test.cpp
                         ranker_test.cpp
                         ir_eval_test.cpp
                         binary_io_test.cpp
                         graph_test.cpp
                         parser_test.cpp
                         language_model_test.cpp
                         filesystem_test.cpp
                         feature_selection_test.cpp
                         tokenizer_filter_test.cpp
                         metadata_test.cpp)
add_dependencies(unit-test ceeaus breast-cancer)
target_link_libraries(unit-test meta-index meta-classify meta-parser
    meta-features meta-language-model)

set(UNIT_TEST_EXE unit-test)
include(unit_tests.cmake)
