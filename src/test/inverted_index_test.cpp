/**
 * @file inverted_index_test.cpp
 * @author Sean Massung
 */

#include "test/inverted_index_test.h"

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
    if (!stop_words)
        throw std::runtime_error{"\"stop-words\" not in config"};

    auto libsvm_modules = orig_config.get_as<std::string>("libsvm-modules");
    if (!libsvm_modules)
        throw std::runtime_error{"\"libsvm-modules\" not in config"};

    auto query_judgements = orig_config.get_as<std::string>("query-judgements");
    if (!query_judgements)
        throw std::runtime_error{"\"query-judgements\" not in config"};

    auto punctuation = orig_config.get_as<std::string>("punctuation");
    if (!punctuation)
        throw std::runtime_error{"\"punctuation\" not in config"};

    auto start_exeptions = orig_config.get_as<std::string>("start-exceptions");
    if (!start_exeptions)
        throw std::runtime_error{"\"start-exceptions\" not in config"};

    auto end_exceptions = orig_config.get_as<std::string>("end-exceptions");
    if (!end_exceptions)
        throw std::runtime_error{"\"end-exceptions\" not in config"};

    config_file << "stop-words = \"" << *stop_words << "\"\n"
                << "punctuation = \"" << *punctuation << "\"\n"
                << "start-exceptions = \"" << *start_exeptions << "\"\n"
                << "end-exceptions = \"" << *end_exceptions << "\"\n"
                << "prefix = \"" << *orig_config.get_as<std::string>("prefix")
                << "\"\n"
                << "query-judgements = \"" << *query_judgements << "\"\n"
                << "libsvm-modules = \"" << *libsvm_modules << "\"\n"
                << "corpus-type = \"" << corpus_type << "-corpus\"\n"
                << "list= \"ceeaus\"\n"
                << "dataset = \"ceeaus\"\n"
                << "encoding = \"shift_jis\"\n"
                << "forward-index = \"ceeaus-fwd\"\n"
                << "inverted-index = \"ceeaus-inv\"\n"
                << "[[analyzers]]\n"
                << "method = \"ngram-word\"\n"
                << "ngram = 1\n"
                << "filter = \"default-chain\"";
}

template <class Index>
void check_ceeaus_expected(Index& idx)
{
    double epsilon = 0.001;
    ASSERT_EQUAL(idx.num_docs(), 1008ul);
    ASSERT_LESS(std::abs(idx.avg_doc_length() - 128.236), epsilon);
    ASSERT_EQUAL(idx.unique_terms(), 3944ul);

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
    ASSERT_EQUAL(idx.doc_freq(t_id), 69ul);

    term_id first;
    double second;
    std::ifstream in{"../data/ceeaus-term-count.txt"};
    auto pdata = idx.search_primary(t_id);
    for (auto& count : pdata->counts())
    {
        in >> first;
        in >> second;
        ASSERT_EQUAL(first, count.first);
        ASSERT_APPROX_EQUAL(second, count.second);
    }
}

int inverted_index_tests()
{
    create_config("file");

    int num_failed = 0;
    num_failed += testing::run_test("inverted-index-build-file-corpus", [&]()
                                    {
        system("rm -rf ceeaus-inv");
        auto idx
            = index::make_index<index::inverted_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected(*idx);
    });

    num_failed += testing::run_test("inverted-index-read-file-corpus", [&]()
                                    {
        {
            auto idx = index::make_index<index::inverted_index,
                                         caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
        }
        system("rm -rf ceeaus-inv test-config.toml");
    });

    create_config("line");
    system("rm -rf ceeaus-inv");

    num_failed += testing::run_test("inverted-index-build-line-corpus", [&]()
                                    {
        auto idx
            = index::make_index<index::inverted_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected(*idx);
    });

    num_failed += testing::run_test("inverted-index-read-line-corpus", [&]()
                                    {
        auto idx
            = index::make_index<index::inverted_index, caching::splay_cache>(
                "test-config.toml", uint32_t{10000});
        check_ceeaus_expected(*idx);
        check_term_id(*idx);
        check_term_id(*idx); // twice to check splay_caching
    });

#if META_HAS_ZLIB
    create_config("gz");
    system("rm -rf ceeaus-inv");

    num_failed += testing::run_test("inverted-index-build-gz-corpus", [&]()
                                    {
        auto idx
            = index::make_index<index::inverted_index, caching::splay_cache>(
                "test-config.toml", 10000);
        check_ceeaus_expected(*idx);
    });

    num_failed += testing::run_test("inverted-index-read-gz-corpus", [&]()
                                    {
        auto idx
            = index::make_index<index::inverted_index, caching::splay_cache>(
                "test-config.toml", 10000);
        check_ceeaus_expected(*idx);
        check_term_id(*idx);
    });
#endif

    // test different caches

    num_failed += testing::run_test("inverted-index-dblru-cache", [&]()
                                    {
        auto idx = index::make_index<index::inverted_index,
                                     caching::default_dblru_cache>(
            "test-config.toml", uint64_t{1000});
        check_term_id(*idx);
        check_term_id(*idx);
    });

    num_failed += testing::run_test("inverted-index-no-evict-cache", [&]()
                                    {
        auto idx
            = index::make_index<index::inverted_index, caching::no_evict_cache>(
                "test-config.toml");
        check_term_id(*idx);
        check_term_id(*idx);
    });

    num_failed += testing::run_test("inverted-index-shard-cache", [&]()
                                    {
        auto idx = index::make_index<index::inverted_index,
                                     caching::splay_shard_cache>(
            "test-config.toml", uint8_t{8});
        check_term_id(*idx);
        check_term_id(*idx);
    });

    system("rm -rf ceeaus-inv test-config.toml");
    return num_failed;
}
}
}
