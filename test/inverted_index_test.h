/**
 * @file inverted_index_test.h
 * @author Sean Massung
 */

#ifndef _INVERTED_INDEX_TEST_H_
#define _INVERTED_INDEX_TEST_H_

#include <fstream>
#include <iostream>
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "caching/all.h"
#include "cpptoml.h"

namespace meta
{
namespace testing
{

void create_config(const std::string& corpus_type)
{
    auto orig_config = cpptoml::parse_file("config.toml");
    std::string config_filename{"test-config.toml"};
    std::ofstream config_file{config_filename};

    auto stop_words = orig_config.get_as<std::string>("stop-words");
    if(!stop_words)
        throw std::runtime_error{"\"stop-words\" not in config"};

    auto libsvm_modules = orig_config.get_as<std::string>("libsvm-modules");
    if(!libsvm_modules)
        throw std::runtime_error{"\"libsvm-modules\" not in config"};

    auto query_judgements = orig_config.get_as<std::string>("query-judgements");
    if(!query_judgements)
        throw std::runtime_error{"\"query-judgements\" not in config"};

    config_file << "stop-words = \"" << *stop_words
                << "\"\n"
                << "prefix = \"" << *orig_config.get_as<std::string>("prefix")
                << "\"\n"
                << "query-judgements = \"" << *query_judgements
                << "\"\n"
                << "libsvm-modules = \"" << *libsvm_modules
                << "\"\n"
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
void check_ceeaus_expected(Index& idx)
{
    double epsilon = 0.000001;
    ASSERT_EQUAL(idx.num_docs(), 1008);
    ASSERT(abs(idx.avg_doc_length() - 128.879) < epsilon);
    ASSERT_EQUAL(idx.unique_terms(), 4003);

    std::ifstream in{"../data/ceeaus-metadata.txt"};
    uint64_t size;
    uint64_t unique;
    doc_id id{0};
    while (in >> size >> unique)
    {
        ASSERT_EQUAL(idx.doc_size(id), size);
        ASSERT_EQUAL(idx.unique_terms(id), unique);
        ++id;
    }

    // make sure there's exactly the correct amount
    ASSERT_EQUAL(id, idx.num_docs());
}

template <class Index>
void check_term_id(Index& idx)
{
    term_id t_id = idx.get_term_id("japanes");
    ASSERT_EQUAL(idx.doc_freq(t_id), 69);

    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-term-count.txt"};
    auto pdata = idx.search_primary(t_id);
    for (auto& count : pdata->counts())
    {
        in >> first;
        in >> second;
        ASSERT_EQUAL(first, count.first);
        ASSERT_EQUAL(second, count.second);
    }
}

int inverted_index_tests()
{
    create_config("file");

    int num_failed = 0;
    num_failed += testing::run_test("inverted-index-build-file-corpus", 30,
    [&]()
    {
        system("/usr/bin/rm -rf ceeaus-inv");
        auto idx = index::make_index
            <index::inverted_index, caching::splay_cache>("test-config.toml",
                                                          uint32_t{10000});
        check_ceeaus_expected(idx);
    });

    num_failed += testing::run_test("inverted-index-read-file-corpus", 10, [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::splay_cache>("test-config.toml",
                                                          uint32_t{10000});
        check_ceeaus_expected(idx);
        check_term_id(idx);
        system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
    });

    create_config("line");
    system("/usr/bin/rm -rf ceeaus-inv");

    num_failed += testing::run_test("inverted-index-build-line-corpus", 30,
    [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::splay_cache>("test-config.toml",
                                                          uint32_t{10000});
        check_ceeaus_expected(idx);
    });

    num_failed += testing::run_test("inverted-index-read-line-corpus", 10, [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::splay_cache>("test-config.toml",
                                                          uint32_t{10000});
        check_ceeaus_expected(idx);
        check_term_id(idx);
        check_term_id(idx); // twice to check splay_caching
    });

    // test different caches

    num_failed += testing::run_test("inverted-index-dblru-cache", 5, [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::default_dblru_cache>(
                "test-config.toml", uint64_t{1000});
        check_term_id(idx);
        check_term_id(idx);
    });

    num_failed += testing::run_test("inverted-index-no-evict-cache", 5, [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::no_evict_cache>(
                "test-config.toml");
        check_term_id(idx);
        check_term_id(idx);
    });

    num_failed += testing::run_test("inverted-index-shard-cache", 5, [&]()
    {
        auto idx = index::make_index
            <index::inverted_index, caching::splay_shard_cache>(
                "test-config.toml", uint8_t{8});
        check_term_id(idx);
        check_term_id(idx);
    });

    system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
    testing::report(num_failed);
    return num_failed;
}
}
}

#endif
