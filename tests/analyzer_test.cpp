/**
 * @file analyzer_test.cpp
 * @author Sean Massung
 */

#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
#include "bandit/bandit.h"
#include "meta/corpus/document.h"
#include "create_config.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"

using namespace bandit;
using namespace meta;

namespace {

std::unique_ptr<analyzers::token_stream> make_filter() {
    using namespace analyzers;
    auto line_cfg = tests::create_config("line");
    return analyzers::default_filter_chain(*line_cfg);
}

template <class Analyzer>
void check_analyzer_expected(Analyzer& ana, corpus::document doc,
                             uint64_t num_unique, uint64_t length) {
    auto counts = ana.template analyze<uint64_t>(doc);
    AssertThat(counts.size(), Equals(num_unique));

    auto total = std::accumulate(
        counts.begin(), counts.end(), uint64_t{0},
        [](uint64_t acc,
           const hashing::kv_pair<std::string, uint64_t>& count) {
            return acc + count.value();
        });

    AssertThat(total, Equals(length));
    AssertThat(doc.id(), Equals(47ul));
}
}

go_bandit([]() {

    corpus::document doc{doc_id{47}};

    describe("[analyzers]: string content", [&]() {

        // "one" is a stopword
        std::string content = "one one two two two three four one five";
        doc.content(content);

        it("should tokenize unigrams from a string", [&]() {
            analyzers::ngram_word_analyzer ana{1, make_filter()};
            check_analyzer_expected(ana, doc, 6, 8);
        });

        it("should tokenize bigrams from a string", [&]() {
            analyzers::ngram_word_analyzer ana{2, make_filter()};
            check_analyzer_expected(ana, doc, 6, 7);
        });

        it("should tokenize trigrams from a string", [&]() {
            analyzers::ngram_word_analyzer ana{3, make_filter()};
            check_analyzer_expected(ana, doc, 6, 6);
        });
    });

    describe("[analyzers]: file content", [&]() {

        doc.content(filesystem::file_text("../data/sample-document.txt"));

        it("should tokenize unigrams from a file", [&]() {
            analyzers::ngram_word_analyzer ana{1, make_filter()};
            check_analyzer_expected(ana, doc, 93, 168);
        });

        it("should tokenize bigrams from a file", [&]() {
            analyzers::ngram_word_analyzer ana{2, make_filter()};
            check_analyzer_expected(ana, doc, 140, 167);
        });

        it("should tokenize trigrams from a file", [&]() {
            analyzers::ngram_word_analyzer ana{3, make_filter()};
            check_analyzer_expected(ana, doc, 159, 166);
        });
    });

    describe("[analyzers]: create from factory", [&]() {
        doc.content(filesystem::file_text("../data/sample-document.txt"));

        it("should create an analyzer from a config object", [&]() {
            auto config = tests::create_config("line");
            auto ana = analyzers::load(*config);
            check_analyzer_expected(*ana, doc, 93, 168);
        });

        it("should create a multi_analyzer from a config object", [&]() {
            auto config = tests::create_config("line", true);
            auto ana = analyzers::load(*config);
            check_analyzer_expected(*ana, doc, 93 + 159, 168 + 166);
        });
    });
});
