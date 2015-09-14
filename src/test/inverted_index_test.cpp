/**
 * @file inverted_index_test.cpp
 * @author Sean Massung
 */

#include "test/inverted_index_test.h"
#include "io/filesystem.h"

namespace meta
{
namespace testing
{

std::shared_ptr<cpptoml::table> create_config(const std::string& corpus_type)
{
    auto orig_config = cpptoml::parse_file("config.toml");
    std::ofstream config_file{"test-config.toml"};

    auto stop_words = orig_config->get_as<std::string>("stop-words");
    if (!stop_words)
        throw std::runtime_error{"\"stop-words\" not in config"};

    auto libsvm_modules = orig_config->get_as<std::string>("libsvm-modules");
    if (!libsvm_modules)
        throw std::runtime_error{"\"libsvm-modules\" not in config"};

    auto punctuation = orig_config->get_as<std::string>("punctuation");
    if (!punctuation)
        throw std::runtime_error{"\"punctuation\" not in config"};

    auto start_exeptions = orig_config->get_as<std::string>("start-exceptions");
    if (!start_exeptions)
        throw std::runtime_error{"\"start-exceptions\" not in config"};

    auto end_exceptions = orig_config->get_as<std::string>("end-exceptions");
    if (!end_exceptions)
        throw std::runtime_error{"\"end-exceptions\" not in config"};

    auto table = cpptoml::make_table();
    table->insert("stop-words", *stop_words);
    table->insert("punctuation", *punctuation);
    table->insert("start-exceptions", *start_exeptions);
    table->insert("end-exceptions", *end_exceptions);
    table->insert("prefix", *orig_config->get_as<std::string>("prefix"));
    table->insert("query-judgements", "../data/ceeaus-qrels.txt");
    table->insert("libsvm-modules", *libsvm_modules);
    table->insert("dataset", "ceeaus");
    table->insert("corpus", corpus_type + ".toml");
    table->insert("encoding", "shift_jis");
    table->insert("forward-index", "ceeaus-fwd");
    table->insert("inverted-index", "ceeaus-inv");

    auto anas = cpptoml::make_table_array();
    auto ana = cpptoml::make_table();
    ana->insert("method", "ngram-word");
    ana->insert<int64_t>("ngram", 1);
    ana->insert("filter", "default-chain");
    anas->push_back(ana);

    table->insert("analyzers", anas);

    auto lm = cpptoml::make_table();
    lm->insert("arpa-file", "../data/english-sentences.arpa");
    lm->insert("binary-file-prefix", "test-lm-");

    table->insert("language-model", lm);

    return table;
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
    auto file_cfg = create_config("file");

    int num_failed = 0;
    num_failed += testing::run_test(
        "inverted-index-build-file-corpus", [&]()
        {
            filesystem::remove_all("ceeaus-inv");
            auto idx = index::make_index<index::inverted_index>(*file_cfg);
            check_ceeaus_expected(*idx);
        });

    num_failed += testing::run_test(
        "inverted-index-read-file-corpus", [&]()
        {
            {
                auto idx = index::make_index<index::inverted_index>(*file_cfg);
                check_ceeaus_expected(*idx);
                check_term_id(*idx);
            }
        });

    auto line_cfg = create_config("line");
    filesystem::remove_all("ceeaus-inv");

    num_failed += testing::run_test(
        "inverted-index-build-line-corpus", [&]()
        {
            auto idx = index::make_index<index::inverted_index>(*line_cfg);
            check_ceeaus_expected(*idx);
        });

    num_failed += testing::run_test(
        "inverted-index-read-line-corpus", [&]()
        {
            auto idx = index::make_index<index::inverted_index,
                                         caching::splay_cache>(*line_cfg,
                                                               uint32_t{10000});
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
            check_term_id(*idx); // twice to check splay_caching
        });

#if META_HAS_ZLIB
    auto gz_cfg = create_config("gz");
    filesystem::remove_all("ceeaus-inv");

    num_failed += testing::run_test(
        "inverted-index-build-gz-corpus", [&]()
        {
            auto idx = index::make_index<index::inverted_index>(*gz_cfg);
            check_ceeaus_expected(*idx);
        });

    num_failed += testing::run_test(
        "inverted-index-read-gz-corpus", [&]()
        {
            auto idx = index::make_index<index::inverted_index>(*gz_cfg);
            check_ceeaus_expected(*idx);
            check_term_id(*idx);
        });
#endif

    // test different caches

    num_failed += testing::run_test(
        "inverted-index-dblru-cache", [&]()
        {
            auto idx = index::make_index<index::inverted_index,
                                         caching::default_dblru_cache>(
                *line_cfg, uint64_t{1000});
            check_term_id(*idx);
            check_term_id(*idx);
        });

    num_failed += testing::run_test(
        "inverted-index-no-evict-cache", [&]()
        {
            auto idx = index::make_index<index::inverted_index,
                                         caching::no_evict_cache>(*line_cfg);
            check_term_id(*idx);
            check_term_id(*idx);
        });

    num_failed += testing::run_test(
        "inverted-index-shard-cache", [&]()
        {
            auto idx = index::make_index<index::inverted_index,
                                         caching::splay_shard_cache>(
                *line_cfg, uint8_t{8});
            check_term_id(*idx);
            check_term_id(*idx);
        });

    filesystem::remove_all("ceeaus-inv");
    return num_failed;
}
}
}
