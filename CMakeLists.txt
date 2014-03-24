cmake_minimum_required(VERSION 2.8.3)
project(meta)

option(USE_LIBCXX "Use libc++ for the C++ standard library" ON)

include_directories(include/
                    deps/cpptoml/include/
                    deps/porter2_stemmer/)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/findicu)
find_package(Threads REQUIRED)
find_package(ICU COMPONENTS data i18n uc REQUIRED)
include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl")
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -ldl")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if(USE_LIBCXX)
            message("-- Locating libc++...")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            find_library(META_LIBCXX NAMES c++ cxx)
            if(META_LIBCXX)
                message("-- Located libc++, using it.")
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
                set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lc++")

                message("--     Locating libc++'s abi...")
                find_library(META_LIBCXXABI NAMES c++abi)
                find_library(META_LIBCXXRT NAMES cxxrt)
                if(META_LIBCXXABI)
                    message("--     Found libc++abi, using it.")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
                    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lc++abi")
                elseif(META_LIBCXXRT)
                    message("--     Found libcxxrt, using it.")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcxxrt")
                    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lcxxrt")
                else()
                    message("--     No abi library found. "
                            "Attempting to continue without one...")
                endif()
            else()
                message("-- Could not find libc++, will not use it.")
            endif()
        endif()
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1)
    add_definitions(-DMETA_IS_DARWIN=1)
endif()

include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
check_cxx_source_compiles("
#include <iomanip>
int main() {
    std::tm t;
    std::put_time(&t, \"%Y\");
    return 0;
}" META_HAS_STD_PUT_TIME)

if(META_HAS_STD_PUT_TIME)
    add_definitions(-DMETA_HAS_STD_PUT_TIME=1)
    add_definitions(-DCPPTOML_HAS_STD_PUT_TIME=1)
endif()

check_cxx_source_runs("
#include <cstring>
#include <ctime>
#include <regex>
int main() {
    std::string to_match = \"2014-02-05T14:30:05Z\";
    std::regex pattern{\"(\\\\\\\\d{4})-(\\\\\\\\d{2})-(\\\\\\\\d{2})T(\\\\\\\\d{2}):(\\\\\\\\d{2}):(\\\\\\\\d{2})Z\"};
    std::match_results<std::string::const_iterator> results;
    std::regex_match(to_match, results, pattern);

    std::tm date;
    std::memset( &date, '\\\\0', sizeof( date ) );
    date.tm_year = stoi( results[1] ) - 1900;
    date.tm_mon = stoi( results[2] ) - 1;
    date.tm_mday = stoi( results[3] );
    date.tm_hour = stoi( results[4] );
    date.tm_min = stoi( results[5] );
    date.tm_sec = stoi( results[6] );
    return 0;
}" META_HAS_STD_REGEX)

if(META_HAS_STD_REGEX)
    add_definitions(-DCPPTOML_HAS_STD_REGEX=1)
endif()

check_cxx_source_compiles("
#include <atomic>
#include <memory>
int main () {
    auto sp = std::make_shared<int>(1);
    auto sp2 = std::atomic_load(&sp);
    return 0;
}" META_HAS_STD_SHARED_PTR_ATOMICS)

if(META_HAS_STD_SHARED_PTR_ATOMICS)
    add_definitions(-DMETA_HAS_STD_SHARED_PTR_ATOMICS=1)
endif()

check_cxx_source_compiles("
#include <fstream>
int main() {
    std::ofstream ofs{\"path\"};
    std::ofstream ofs2{std::move(ofs)};
    return 0;
}" META_HAS_STREAM_MOVE)

if(META_HAS_STREAM_MOVE)
    add_definitions(-DMETA_HAS_STREAM_MOVE=1)
endif()

add_library(meta-util
            src/util/progress.cpp)

add_library(meta-utf
            src/utf/segmenter.cpp
            src/utf/transformer.cpp
            src/utf/utf.cpp)
target_link_libraries(meta-utf ${ICU_LIBRARIES})

add_library(meta-io
            src/io/compressed_file_reader.cpp
            src/io/parser.cpp
            src/io/libsvm_parser.cpp
            src/io/compressed_file_writer.cpp
            src/io/mmap_file.cpp)
target_link_libraries(meta-io meta-util)

add_library(meta-corpus
            src/corpus/document.cpp
            src/corpus/file_corpus.cpp
            src/corpus/line_corpus.cpp
            src/corpus/corpus.cpp)

# some corpus classes use io::parser
target_link_libraries(meta-corpus meta-io)

add_library(meta-stemmers
            deps/porter2_stemmer/porter2_stemmer.cpp)

add_library(meta-analyzers
            src/analyzers/filters/alpha_filter.cpp
            src/analyzers/filters/empty_sentence_filter.cpp
            src/analyzers/filters/english_normalizer.cpp
            src/analyzers/filters/icu_filter.cpp
            src/analyzers/filters/length_filter.cpp
            src/analyzers/filters/list_filter.cpp
            src/analyzers/filters/lowercase_filter.cpp
            src/analyzers/filters/porter2_stemmer.cpp
            src/analyzers/filters/sentence_boundary.cpp
            src/analyzers/tokenizers/character_tokenizer.cpp
            src/analyzers/tokenizers/icu_tokenizer.cpp
            src/analyzers/tokenizers/whitespace_tokenizer.cpp
            src/analyzers/filter_factory.cpp
            src/analyzers/analyzer.cpp
            src/analyzers/libsvm_analyzer.cpp
            src/analyzers/multi_analyzer.cpp
            src/analyzers/ngram/ngram_lex_analyzer.cpp
            src/analyzers/ngram/ngram_pos_analyzer.cpp
            src/analyzers/ngram/ngram_simple_analyzer.cpp
            src/analyzers/ngram/ngram_analyzer.cpp
            src/analyzers/ngram/ngram_word_analyzer.cpp
            src/analyzers/tree/branch_analyzer.cpp
            src/analyzers/tree/depth_analyzer.cpp
            src/analyzers/tree/parse_tree.cpp
            src/analyzers/tree/semi_skeleton_analyzer.cpp
            src/analyzers/tree/skeleton_analyzer.cpp
            src/analyzers/tree/subtree_analyzer.cpp
            src/analyzers/tree/tag_analyzer.cpp
            src/analyzers/analyzer_factory.cpp)

target_link_libraries(meta-analyzers meta-corpus meta-stemmers meta-utf)

add_library(meta-index
            src/index/disk_index.cpp
            src/index/inverted_index.cpp
            src/index/forward_index.cpp
            src/index/string_list.cpp
            src/index/string_list_writer.cpp
            src/index/vocabulary_map.cpp
            src/index/vocabulary_map_writer.cpp
            src/index/eval/ir_eval.cpp
            src/index/ranker/okapi_bm25.cpp
            src/index/ranker/lm_ranker.cpp
            src/index/ranker/ranker.cpp
            src/index/ranker/jelinek_mercer.cpp
            src/index/ranker/dirichlet_prior.cpp
            src/index/ranker/absolute_discount.cpp
            src/index/ranker/pivoted_length.cpp
            src/index/ranker/ranker_factory.cpp)

target_link_libraries(meta-index meta-analyzers ${CMAKE_THREAD_LIBS_INIT})

add_library(meta-classify
            src/classify/confusion_matrix.cpp
            src/classify/classifier_factory.cpp
            src/classify/binary_classifier_factory.cpp
            src/classify/loss/loss_function_factory.cpp
            src/classify/classifier/classifier.cpp
            src/classify/classifier/binary_classifier.cpp
            src/classify/classifier/sgd.cpp
            src/classify/classifier/naive_bayes.cpp
            src/classify/classifier/svm_wrapper.cpp
            src/classify/classifier/winnow.cpp
            src/classify/classifier/one_vs_all.cpp
            src/classify/classifier/one_vs_one.cpp
            src/classify/classifier/dual_perceptron.cpp
            src/classify/classifier/knn.cpp
            src/classify/loss/hinge.cpp
            src/classify/loss/huber.cpp
            src/classify/loss/least_squares.cpp
            src/classify/loss/logistic.cpp
            src/classify/loss/modified_huber.cpp
            src/classify/loss/perceptron.cpp
            src/classify/loss/smooth_hinge.cpp
            src/classify/loss/squared_hinge.cpp)
target_link_libraries(meta-classify meta-index)

add_library(meta-topics
            src/topics/lda_cvb.cpp
            src/topics/lda_gibbs.cpp
            src/topics/lda_model.cpp
            src/topics/parallel_lda_gibbs.cpp)
target_link_libraries(meta-topics meta-index)

add_library(meta-sequence
            src/sequence/sequence.cpp
            src/sequence/observation.cpp)
target_link_libraries(meta-sequence meta-util)

add_library(meta-testing
            src/test/classifier_test.cpp
            src/test/forward_index_test.cpp
            src/test/inverted_index_test.cpp
            src/test/ir_eval_test.cpp
            src/test/libsvm_parser_test.cpp
            src/test/parallel_test.cpp
            src/test/ranker_test.cpp
            src/test/stemmer_test.cpp
            src/test/string_list_test.cpp
            src/test/analyzer_test.cpp
            src/test/filter_test.cpp
            src/test/compression_test.cpp
            src/test/vocabulary_map_test.cpp)
target_link_libraries(meta-testing meta-index meta-classify)

add_executable(corpus-gen corpus-gen.cpp)
target_link_libraries(corpus-gen meta-corpus)

add_executable(classify classify.cpp)
target_link_libraries(classify meta-classify)

add_executable(online-classify online-classify.cpp)
target_link_libraries(online-classify meta-classify)

add_executable(unit-test unit-test.cpp)
target_link_libraries(unit-test meta-testing meta-classify meta-index)

add_executable(query-runner query-runner.cpp)
target_link_libraries(query-runner meta-index)

add_executable(search search.cpp)
target_link_libraries(search meta-index)

add_executable(index index.cpp)
target_link_libraries(index meta-index)

add_executable(interactive-search interactive-search.cpp)
target_link_libraries(interactive-search meta-index)

add_executable(lda lda.cpp)
target_link_libraries(lda meta-topics)

add_executable(lda-topics lda-topics.cpp)
target_link_libraries(lda-topics meta-index)

add_executable(shuffle shuffle.cpp)

add_executable(utf8-test utf8-test.cpp)
target_link_libraries(utf8-test meta-utf)

add_executable(print-vocab print_vocab.cpp)
target_link_libraries(print-vocab meta-index)

add_executable(search-vocab search-vocab.cpp)
target_link_libraries(search-vocab meta-index)

enable_testing()
add_test(analyzers unit-test analyzers)
set_tests_properties(analyzers PROPERTIES TIMEOUT 10)
add_test(stemmers unit-test stemmers)
set_tests_properties(stemmers PROPERTIES TIMEOUT 10)
add_test(parallel unit-test parallel)
set_tests_properties(parallel PROPERTIES TIMEOUT 30)
add_test(inverted-index unit-test inverted-index)
set_tests_properties(inverted-index PROPERTIES TIMEOUT 30)
add_test(forward-index unit-test forward-index)
set_tests_properties(forward-index PROPERTIES TIMEOUT 30)
add_test(string-list unit-test string-list)
set_tests_properties(string-list PROPERTIES TIMEOUT 10)
add_test(vocabulary-map unit-test vocabulary-map)
set_tests_properties(vocabulary-map PROPERTIES TIMEOUT 10)
add_test(libsvm-parser unit-test libsvm-parser)
set_tests_properties(libsvm-parser PROPERTIES TIMEOUT 10)
add_test(classifiers unit-test classifiers)
set_tests_properties(classifiers PROPERTIES TIMEOUT 60)
add_test(rankers unit-test rankers)
set_tests_properties(rankers PROPERTIES TIMEOUT 75)
add_test(ir-eval unit-test ir-eval)
set_tests_properties(ir-eval PROPERTIES TIMEOUT 10)
add_test(compression unit-test compression)
set_tests_properties(compression PROPERTIES TIMEOUT 10)

add_custom_target(tidy
                  COMMAND /usr/bin/rm -rf
                  ./doc
                  *.terms
                  *.phi
                  *.theta)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/meta.doxygen.in
                   ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen @ONLY)
    add_custom_target(doc
                      ${DOXYGEN_EXECUTABLE}
                      ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
