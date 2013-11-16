cmake_minimum_required( VERSION 2.8 )
project( meta )

option( USE_LIBCXX "Use libc++ for the C++ standard library" ON )

include_directories( "include/"
                     "deps/cpptoml/include/"
                     "deps/porter2_stemmer/"
)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/lib" )

find_package( Threads REQUIRED )

if( UNIX )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
    if( USE_LIBCXX )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
    endif()
endif()

add_library( meta-classify
             src/classify/feature_select/select.cpp
             src/classify/feature_select/doc_freq.cpp
             src/classify/feature_select/select_simple.cpp
             src/classify/confusion_matrix.cpp
             src/classify/feature_select/chi_square.cpp
             src/classify/feature_select/info_gain.cpp
             src/classify/feature_select/correlation.cpp
             src/classify/feature_select/odds.cpp
             src/classify/classifier/naive_bayes.cpp
             src/classify/classifier/svm_wrapper.cpp
             src/classify/classifier/winnow.cpp
)

add_library( meta-corpus
             src/corpus/document.cpp
             src/corpus/file_corpus.cpp
             src/corpus/line_corpus.cpp
             src/corpus/corpus.cpp
)

add_library( meta-index
             src/index/inverted_index.cpp
             src/index/forward_index.cpp
             src/index/ranker/okapi_bm25.cpp
             src/index/ranker/lm_ranker.cpp
             src/index/ranker/ranker.cpp
             src/index/ranker/jelinek_mercer.cpp
             src/index/ranker/dirichlet_prior.cpp
             src/index/ranker/absolute_discount.cpp
             src/index/ranker/pivoted_length.cpp
)

add_library( meta-io
             src/io/compressed_file_reader.cpp
             src/io/parser.cpp
             src/io/compressed_file_writer.cpp
             src/io/mmap_file.cpp
             src/io/file_writer.cpp
)

add_library( meta-stemmers
             src/stemmers/no_stemmer.cpp
             src/stemmers/porter2.cpp
             deps/porter2_stemmer/porter2_stemmer.cpp
)

add_library( meta-tokenizers
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
             src/tokenizers/tree/tag_tokenizer.cpp
)

add_library( meta-topics
             src/topics/lda_cvb.cpp
             src/topics/lda_gibbs.cpp
             src/topics/lda_model.cpp
             src/topics/slda.cpp
             src/topics/parallel_lda_gibbs.cpp
)

add_executable( classify classify-test.cpp )
target_link_libraries( classify
                       meta-classify
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-io
                       meta-corpus
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( index index.cpp )
target_link_libraries( index
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-io
                       meta-corpus
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( search search.cpp )
target_link_libraries( search
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-io
                       meta-corpus
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( interactive_search interactive_search.cpp )
target_link_libraries( interactive_search
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-io
                       meta-corpus
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( threadpool-test threadpool-test.cpp )
target_link_libraries( threadpool-test
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( parallel-for-test parallel-for-test.cpp )
target_link_libraries( parallel-for-test
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( lda-test lda-test.cpp )
target_link_libraries( lda-test
                       meta-topics
                       meta-tokenizers
                       meta-stemmers
                       meta-index
                       meta-io
                       meta-corpus
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( lda-topics lda-topics.cpp )
target_link_libraries( lda-topics
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( shuffle shuffle.cpp )

add_custom_target( tidy
                   COMMAND /usr/bin/rm -rf
                   ./doc
                   *.terms
                   *.phi
                   *.theta
)

find_package( Doxygen )
if( DOXYGEN_FOUND )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/meta.doxygen.in
                    ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen @ONLY
    )
    add_custom_target( doc
                       ${DOXYGEN_EXECUTABLE}
                       ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

file( GLOB INI_FILES *.ini )
file( COPY ${INI_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
