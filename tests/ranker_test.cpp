/**
 * @file ranker_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/corpus/document.h"
#include "meta/index/ranker/all.h"

using namespace bandit;
using namespace meta;

namespace {

template <class Ranker, class Index>
void test_rank(Ranker& r, Index& idx, const std::string& encoding) {
    // exhaustive search for each document
    for (size_t i = 0; i < idx.num_docs(); ++i) {
        auto d_id = idx.docs()[i];
        auto path = idx.doc_path(d_id);
        corpus::document query{doc_id{i}};
        query.content(filesystem::file_text(path), encoding);

        auto ranking = r.score(idx, query);
        AssertThat(ranking.size(), Equals(10ul)); // default is 10 docs

        // since we're searching for a document already in the index, the same
        // document should be ranked first, but there are a few duplicate
        // documents......
        if (ranking[0].d_id != i) {
            AssertThat(ranking[1].d_id, Equals(i));
            AssertThat(ranking[0].score,
                       EqualsWithDelta(ranking[1].score, 0.0001));
        }
    }

    // sanity checks for simple query
    corpus::document query;
    query.content("character");

    auto ranking = r.score(idx, query);
    // ensure there is diversity in the top 10 documents
    AssertThat(ranking[0].score, Is().GreaterThan(ranking.back().score));

    // check for sorted-ness of ranking
    for (uint64_t i = 1; i < ranking.size(); ++i) {
        AssertThat(ranking[i - 1].score,
                   Is().GreaterThanOrEqualTo(ranking[i].score));
    }
}
}

go_bandit([]() {

    describe("[rankers]", []() {

        auto config = tests::create_config("file");
        filesystem::remove_all("ceeaus");
        auto idx = index::make_index<index::inverted_index>(*config);
        std::string encoding = "utf-8";
        if (auto enc = config->get_as<std::string>("encoding"))
            encoding = *enc;

        it("should be able to rank with absolute discounting", [&]() {
            index::absolute_discount r;
            test_rank(r, *idx, encoding);
        });

        it("should be able to rank with Dirichlet prior", [&]() {
            index::dirichlet_prior r;
            test_rank(r, *idx, encoding);
        });

        it("should be able to rank with Jelinek-Mercer", [&]() {
            index::jelinek_mercer r;
            test_rank(r, *idx, encoding);
        });

        it("should be able to rank with Okapi BM25", [&]() {
            index::okapi_bm25 r;
            test_rank(r, *idx, encoding);
        });

        it("should be able to rank with pivoted length normalization", [&]() {
            index::pivoted_length r;
            test_rank(r, *idx, encoding);
        });

        idx = nullptr;
        filesystem::remove_all("ceeaus");
    });
});
