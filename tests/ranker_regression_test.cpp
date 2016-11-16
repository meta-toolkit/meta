/**
 * @file ranker_regression_test.cpp
 * @author Chase Geigle
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/corpus/document.h"
#include "meta/index/eval/ir_eval.h"
#include "meta/index/forward_index.h"
#include "meta/index/ranker/all.h"

using namespace bandit;
using namespace meta;

namespace {
struct ret_perf {
    double map;
    double avg_ndcg;
};

ret_perf retrieval_performance(index::ranker& r, index::inverted_index& idx,
                               const cpptoml::table& cfg) {
    index::ir_eval eval{cfg};

    std::ifstream queries{*cfg.get_as<std::string>("query-path")};
    std::string line;

    double cumulative_ndcg = 0.0;
    uint64_t num_queries = 0;
    for (query_id qid{1}; std::getline(queries, line); ++qid, ++num_queries) {
        corpus::document query;
        query.content(line);
        auto results = r.score(idx, query, 1);
        eval.avg_p(results, qid, results.size());
        cumulative_ndcg += eval.ndcg(results, qid, results.size());
    }

    ret_perf perf;
    perf.map = eval.map();
    perf.avg_ndcg = cumulative_ndcg / num_queries;
    return perf;
}
}

go_bandit([]() {

    describe("[ranker regression]", []() {
        auto cfg = tests::create_config("line");
        cfg->insert("dataset", "cranfield");
        cfg->insert("query-judgements",
                    "../data/cranfield/cranfield-qrels.txt");
        cfg->insert("index", "cranfield-idx");
        cfg->insert("query-path", "../data/cranfield/cranfield-queries.txt");

        auto anas = cfg->get_table_array("analyzers");
        auto ana = anas->get()[0];
        ana->insert("filter", "default-unigram-chain");

        filesystem::remove_all("cranfield-idx");
        auto idx = index::make_index<index::inverted_index>(*cfg);

        it("should obtain expected performance with absolute discounting",
           [&]() {
               index::absolute_discount r;
               auto perf = retrieval_performance(r, *idx, *cfg);
               AssertThat(perf.map, IsGreaterThan(0.34));
               AssertThat(perf.avg_ndcg, IsGreaterThan(0.22));
           });

        it("should obtain expected performance with Dirichlet prior", [&]() {
            index::dirichlet_prior r;
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.30));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.21));
        });

        it("should obtain expected performance with Jelinek-Mercer", [&]() {
            index::jelinek_mercer r;
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.34));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.23));
        });

        it("should obtain expected performance with Okapi BM25", [&]() {
            index::okapi_bm25 r;
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.33));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.22));
        });

        it("should obtain expected performance with pivoted length", [&]() {
            index::pivoted_length r;
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.32));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.21));
        });

        it("should obtain expected performance with KL-divergence PRF", [&]() {
            index::kl_divergence_prf r{
                index::make_index<index::forward_index>(*cfg)};
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.33));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.22));
        });

        it("should obtain expected performance with Rocchio", [&]() {
            index::rocchio r{index::make_index<index::forward_index>(*cfg)};
            auto perf = retrieval_performance(r, *idx, *cfg);
            AssertThat(perf.map, IsGreaterThan(0.34));
            AssertThat(perf.avg_ndcg, IsGreaterThan(0.23));
        });

        it("should get better performance than Dirichlet prior when using "
           "KL-divergence PRF",
           [&]() {
               index::kl_divergence_prf kl_div{
                   index::make_index<index::forward_index>(*cfg)};
               auto kl_perf = retrieval_performance(kl_div, *idx, *cfg);

               index::dirichlet_prior dp;
               auto dp_perf = retrieval_performance(dp, *idx, *cfg);

               AssertThat(kl_perf.map, IsGreaterThanOrEqualTo(dp_perf.map));
               AssertThat(kl_perf.avg_ndcg,
                          IsGreaterThanOrEqualTo(dp_perf.avg_ndcg));
           });

        it("should get better performance than Jelinek-Mercer when using "
           "KL-divergence PRF",
           [&]() {
               index::kl_divergence_prf kl_div{
                   index::make_index<index::forward_index>(*cfg),
                   make_unique<index::jelinek_mercer>()};
               auto kl_perf = retrieval_performance(kl_div, *idx, *cfg);

               index::jelinek_mercer jm;
               auto jm_perf = retrieval_performance(jm, *idx, *cfg);

               AssertThat(kl_perf.map, IsGreaterThanOrEqualTo(jm_perf.map));
               AssertThat(kl_perf.avg_ndcg,
                          IsGreaterThanOrEqualTo(jm_perf.avg_ndcg));
           });

        it("should get better performance than Okapi BM25 when using Rocchio",
           [&]() {
               index::rocchio rocchio{
                   index::make_index<index::forward_index>(*cfg),
                   make_unique<index::okapi_bm25>()};

               auto rocchio_perf = retrieval_performance(rocchio, *idx, *cfg);

               index::okapi_bm25 bm25;
               auto bm25_perf = retrieval_performance(bm25, *idx, *cfg);

               AssertThat(rocchio_perf.map,
                          IsGreaterThanOrEqualTo(bm25_perf.map));
               AssertThat(rocchio_perf.avg_ndcg,
                          IsGreaterThanOrEqualTo(bm25_perf.avg_ndcg));
           });

        it("should get better performance than pivoted length when using "
           "Rocchio",
           [&]() {
               index::rocchio rocchio{
                   index::make_index<index::forward_index>(*cfg),
                   make_unique<index::okapi_bm25>()};

               auto rocchio_perf = retrieval_performance(rocchio, *idx, *cfg);

               index::pivoted_length pl;
               auto pl_perf = retrieval_performance(pl, *idx, *cfg);

               AssertThat(rocchio_perf.map,
                          IsGreaterThanOrEqualTo(pl_perf.map));
               AssertThat(rocchio_perf.avg_ndcg,
                          IsGreaterThanOrEqualTo(pl_perf.avg_ndcg));
           });

        idx = nullptr;
        filesystem::remove_all("cranfield-idx");
    });
});
