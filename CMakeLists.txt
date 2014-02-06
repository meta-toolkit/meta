cmake_minimum_required(VERSION 2.8)
project(meta)

option(USE_LIBCXX "Use libc++ for the C++ standard library" ON)

include_directories(include/
                    deps/cpptoml/include/
                    deps/porter2_stemmer/
                    test/)

find_package(Threads REQUIRED)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKDER_FLAGS} -ldl")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if(USE_LIBCXX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
            set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lc++abi")
        endif()
    endif()
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
            src/stemmers/no_stemmer.cpp
            src/stemmers/porter2.cpp
            deps/porter2_stemmer/porter2_stemmer.cpp)

add_library(meta-tokenizers
            src/tokenizers/multi_tokenizer.cpp
            src/tokenizers/tokenizer.cpp
            src/tokenizers/libsvm_tokenizer.cpp
            src/tokenizers/ngram/ngram_char_tokenizer.cpp
            src/tokenizers/ngram/ngram_fw_tokenizer.cpp
            src/tokenizers/ngram/ngram_lex_tokenizer.cpp
            src/tokenizers/ngram/ngram_pos_tokenizer.cpp
            src/tokenizers/ngram/ngram_simple_tokenizer.cpp
            src/tokenizers/ngram/ngram_tokenizer.cpp
            src/tokenizers/ngram/ngram_word_tokenizer.cpp
            src/tokenizers/tree/branch_tokenizer.cpp
            src/tokenizers/tree/depth_tokenizer.cpp
            src/tokenizers/tree/parse_tree.cpp
            src/tokenizers/tree/semi_skeleton_tokenizer.cpp
            src/tokenizers/tree/skeleton_tokenizer.cpp
            src/tokenizers/tree/subtree_tokenizer.cpp
            src/tokenizers/tree/tag_tokenizer.cpp)

# some tokenizer clases use io::parser
# ngram_word_tokenizer needs access to the stemmers
target_link_libraries(meta-tokenizers meta-io meta-corpus meta-stemmers)

add_library(meta-index
            src/index/disk_index.cpp
            src/index/inverted_index.cpp
            src/index/forward_index.cpp
            src/index/string_list.cpp
            src/index/string_list_writer.cpp
            src/index/vocabulary_map.cpp
            src/index/vocabulary_map_writer.cpp
            src/index/ranker/okapi_bm25.cpp
            src/index/ranker/lm_ranker.cpp
            src/index/ranker/ranker.cpp
            src/index/ranker/jelinek_mercer.cpp
            src/index/ranker/dirichlet_prior.cpp
            src/index/ranker/absolute_discount.cpp
            src/index/ranker/pivoted_length.cpp)

target_link_libraries(meta-index
                      meta-util meta-io meta-corpus meta-tokenizers)

add_library(meta-classify
            src/classify/confusion_matrix.cpp
            src/classify/classifier/naive_bayes.cpp
            src/classify/classifier/svm_wrapper.cpp
            src/classify/classifier/winnow.cpp)
target_link_libraries(meta-classify meta-index)

add_library(meta-topics
            src/topics/lda_cvb.cpp
            src/topics/lda_gibbs.cpp
            src/topics/lda_model.cpp
            # src/topics/slda.cpp
            src/topics/parallel_lda_gibbs.cpp)
target_link_libraries(meta-topics meta-index)

add_executable(classify classify-test.cpp)
target_link_libraries(classify
                      meta-util
                      meta-classify
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(unit-test unit-test.cpp)
target_link_libraries(unit-test
                      meta-classify
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(query-runner query-runner.cpp)
target_link_libraries(query-runner
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(search search.cpp)
target_link_libraries(search
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(index index.cpp)
target_link_libraries(index
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(interactive_search interactive_search.cpp)
target_link_libraries(interactive_search
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(lda-test lda-test.cpp)
target_link_libraries(lda-test
                      meta-topics
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(lda-topics lda-topics.cpp)
target_link_libraries(lda-topics
                      meta-index
                      ${CMAKE_THREAD_LIBS_INIT})

add_executable(shuffle shuffle.cpp)

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
