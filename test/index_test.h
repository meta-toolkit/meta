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

    template <class Index>
    void check_ceeaus_expected(Index & idx)
    {
        double epsilon = 0.000001;
        ASSERT(idx.num_docs() == 1008);
        ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
        ASSERT(idx.unique_terms() == 4003);

        std::ifstream in{"../data/ceeaus-metadata.txt"};
        uint64_t size;
        uint64_t unique;
        doc_id id{0};
        while(in >> size >> unique)
        {
            ASSERT(idx.doc_size(id) == size);
            ASSERT(idx.unique_terms(id) == unique);
            ++id;
        }

        // make sure there's exactly the correct amount
        ASSERT(id == idx.num_docs());
    }

    template <class Index>
    void check_doc_id(Index & idx)
    {
        auto pdata = idx.search_primary(term_id{0});
        ASSERT(pdata->primary_key() == term_id{0});
        pdata = idx.search_primary(term_id{2});
        ASSERT(pdata->primary_key() == term_id{2});
        /*
        for(auto & count: pdata->counts())
            std::cout << count.first << ":" << count.second << std::endl;
        */
    }

    void index_tests()
    {
        create_config("file");

        testing::run_test("ceeaus-build-file-corpus", 30, [&](){
            system("/usr/bin/rm -rf ceeaus-inv");
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            check_ceeaus_expected(idx);
        });

        testing::run_test("ceeaus-read-file-corpus", 10, [&](){
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            check_ceeaus_expected(idx);
            check_doc_id(idx);
            system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
        });

        create_config("line");

        testing::run_test("ceeaus-build-line-corpus", 30, [&](){
            system("/usr/bin/rm -rf ceeaus-inv");
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            check_ceeaus_expected(idx);
        });

        testing::run_test("ceeaus-read-line-corpus", 10, [&](){
            auto idx = index::make_index<index::inverted_index,
                caching::splay_cache>("test-config.toml", uint32_t{10000});
            check_ceeaus_expected(idx);
            check_doc_id(idx);
            system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
        });
    }

}
}

#endif
