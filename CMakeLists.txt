cmake_minimum_required( VERSION 2.8 )
project( meta )

option( USE_LIBCXX "Use libc++ for the C++ standard library" ON )

include_directories( "include/" )
include_directories( "lib/slda/" )

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
             src/classify/classifier.cpp
             src/classify/select.cpp
             src/classify/select_doc_freq.cpp
             src/classify/select_simple.cpp
             src/classify/confusion_matrix.cpp
             src/classify/select_chi_square.cpp
             src/classify/select_info_gain.cpp  
             src/classify/knn.cpp
             src/classify/select_corr.cpp
             src/classify/select_odds.cpp
             src/classify/naive_bayes.cpp
             src/classify/liblinear_svm.cpp
)

add_library( meta-index
             src/index/chunk_list.cpp
             src/index/inverted_index.cpp
             src/index/postings.cpp
             src/index/structs.cpp
             src/index/document.cpp
             src/index/lexicon.cpp
             src/index/ram_index.cpp
)

add_library( meta-io
             src/io/compressed_file_reader.cpp
             src/io/config_reader.cpp
             src/io/parser.cpp
             src/io/compressed_file_writer.cpp
             src/io/mmap_file.cpp
)

add_library( meta-stemmers
             src/stemmers/no_stemmer.cpp
             src/stemmers/porter2.cpp
             src/stemmers/porter2_stemmer.cpp
)

add_library( meta-tokenizers
             src/tokenizers/multi_tokenizer.cpp
             src/tokenizers/ngram_lex_tokenizer.cpp
             src/tokenizers/ngram_tokenizer.cpp
             src/tokenizers/tree_tokenizer.cpp
             src/tokenizers/ngram_char_tokenizer.cpp
             src/tokenizers/ngram_pos_tokenizer.cpp
             src/tokenizers/parse_tree.cpp
             src/tokenizers/ngram_fw_tokenizer.cpp
             src/tokenizers/ngram_simple_tokenizer.cpp
             src/tokenizers/tokenizer.cpp
)

add_library( meta-topics
             src/topics/lda_cvb.cpp  
             src/topics/lda_gibbs.cpp
             src/topics/lda_model.cpp
             src/topics/slda.cpp
             src/topics/parallel_lda_gibbs.cpp
)

add_executable( eval eval.cpp )
target_link_libraries( eval
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-classify
                       ${CMAKE_THREAD_LIBS_INIT} 
)

add_executable( lm lm.cpp )
target_link_libraries( lm 
                       meta-io 
                       meta-index
                       meta-tokenizers 
                       meta-stemmers 
                       ${CMAKE_THREAD_LIBS_INIT} 
)

add_executable( classify classify-test.cpp )
target_link_libraries( classify
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-classify
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( slda slda-test.cpp )
target_link_libraries( slda
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-topics
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( search search.cpp )
target_link_libraries( search
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( feature-select feature-select.cpp )
target_link_libraries( feature-select
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       meta-classify
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( cluster-test cluster-test.cpp )
target_link_libraries( cluster-test
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
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
                       meta-io
                       meta-index
                       meta-tokenizers
                       meta-stemmers
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_executable( lda-topics lda-topics.cpp )
target_link_libraries( lda-topics
                       ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_target( tidy
                   COMMAND /usr/bin/rm -rf
                   ./doc
                   *.chunk
                   postingsFile
                   lexiconFile
                   termid.mapping
                   docid.mapping
                   docs.lengths
                   *compressed.txt
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
