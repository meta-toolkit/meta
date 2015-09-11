/**
 * @file analyzer_test.cpp
 * @author Sean Massung
 */

#include "test/analyzer_test.h"
#include "test/inverted_index_test.h"
#include "analyzers/token_stream.h"
#include "corpus/document.h"
#include "util/shim.h"

namespace meta
{
namespace testing
{

namespace
{
std::unique_ptr<analyzers::token_stream> make_filter()
{
    using namespace analyzers;
    create_config("line");
    auto config = cpptoml::parse_file("test-config.toml");
    return analyzers::default_filter_chain(config);
}
}

template <class Analyzer>
void check_analyzer_expected(Analyzer& ana, corpus::document doc,
                             uint64_t num_unique, uint64_t length)
{
    auto counts = ana.analyze(doc);
    ASSERT_EQUAL(counts.size(), num_unique);

    auto total = std::accumulate(
        counts.begin(), counts.end(), uint64_t{0},
        [](uint64_t acc,
           const typename Analyzer::feature_map::value_type& count)
        {
            return acc + count.second;
        });
    ASSERT_EQUAL(total, length);
    ASSERT_EQUAL(doc.id(), 47ul);
}

int content_tokenize()
{
    corpus::document doc{doc_id{47}};

    // "one" is a stopword
    std::string content = "one one two two two three four one five";
    doc.content(content);
    int num_failed = 0;

    num_failed += testing::run_test(
        "content-unigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{1, make_filter()};
            check_analyzer_expected(tok, doc, 6, 8);
        });

    num_failed += testing::run_test(
        "content-bigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{2, make_filter()};
            check_analyzer_expected(tok, doc, 6, 7);
        });

    num_failed += testing::run_test(
        "content-trigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{3, make_filter()};
            check_analyzer_expected(tok, doc, 6, 6);
        });

    return num_failed;
}

int file_tokenize()
{
    int num_failed = 0;
    corpus::document doc{doc_id{47}};
    doc.content(filesystem::file_text("../data/sample-document.txt"));

    num_failed += testing::run_test(
        "file-unigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{1, make_filter()};
            check_analyzer_expected(tok, doc, 93, 168);
        });

    num_failed += testing::run_test(
        "file-bigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{2, make_filter()};
            check_analyzer_expected(tok, doc, 140, 167);
        });

    num_failed += testing::run_test(
        "file-trigram-word-analyzer", [&]()
        {
            analyzers::ngram_word_analyzer<uint64_t> tok{3, make_filter()};
            check_analyzer_expected(tok, doc, 159, 166);
        });

    return num_failed;
}

int analyzer_tests()
{
    int num_failed = 0;
    num_failed += content_tokenize();
    num_failed += file_tokenize();
    return num_failed;
}
}
}
