/**
 * @file index_test.h
 */

#ifndef _INDEX_TEST_H_
#define _INDEX_TEST_H_

#include <fstream>
#include <iostream>
#include "index/inverted_index.h"
#include "caching/all.h"
#include "cpptoml.h"

namespace meta {
namespace testing {

    void create_config(const std::string & corpus_type)
    {
        auto orig_config = cpptoml::parse_file("config.toml");
        std::string config_filename{"test-config.toml"};
        std::ofstream config_file{config_filename};

        config_file << "stop-words = \""
                    << *orig_config.get_as<std::string>("stop-words") << "\"\n"
                    << "prefix = \""
                    << *orig_config.get_as<std::string>("prefix") << "\"\n"
                    << "corpus-type = \"" << corpus_type << "-corpus\"\n"
                    << "list= \"ceeaus\"\n"
                    << "dataset = \"ceeaus\"\n"
                    << "forward-index = \"ceeaus-fwd\"\n"
                    << "inverted-index = \"ceeaus-inv\"\n"
                    << "[[tokenizers]]\n"
                    << "method = \"ngram\"\n"
                    << "ngramOpt = \"Word\"\n"
                    << "ngram = 1\n";
    }

    void index_tests()
    {
        double epsilon = 0.001;
        create_config("file");

        testing::run_test("ceeaus-build-file-corpus", 30, [&](){
            system("/usr/bin/rm -rf ceeaus-inv");
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            ASSERT(idx.num_docs() == 1008);
            ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
            ASSERT(idx.unique_terms() == 4003);
        });
        
        testing::run_test("ceeaus-read-file-corpus", 10, [&](){
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            ASSERT(idx.num_docs() == 1008);
            ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
            ASSERT(idx.unique_terms() == 4003);
            system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
        });

        create_config("line");

        testing::run_test("ceeaus-build-line-corpus", 30, [&](){
            system("/usr/bin/rm -rf ceeaus-inv");
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            ASSERT(idx.num_docs() == 1008);
            ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
            ASSERT(idx.unique_terms() == 4003);
        });
        
        testing::run_test("ceeaus-read-line-corpus", 10, [&](){
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            ASSERT(idx.num_docs() == 1008);
            ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
            ASSERT(idx.unique_terms() == 4003);
            system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
        });
    }

}
}

#endif
