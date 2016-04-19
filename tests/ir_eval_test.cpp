/**
 * @file ir_eval_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "meta/corpus/document.h"
#include "meta/index/eval/ir_eval.h"
#include "meta/index/eval/rank_correlation.h"
#include "meta/index/ranker/okapi_bm25.h"
#include "meta/index/ranker/ranker.h"

using namespace bandit;
using namespace meta;

namespace {

void check_query(index::ir_eval& eval,
                 const std::vector<index::search_result>& ranking, query_id qid,
                 double e_f1, double e_p, double e_r, double e_avg_p,
                 double e_ndcg,
                 uint64_t num_docs = std::numeric_limits<uint64_t>::max()) {
    auto f1 = eval.f1(ranking, qid, num_docs);
    auto p = eval.precision(ranking, qid, num_docs);
    auto r = eval.recall(ranking, qid, num_docs);
    auto avg_p = eval.avg_p(ranking, qid, num_docs);
    auto ndcg = eval.ndcg(ranking, qid, num_docs);
    const double delta = 0.000001;
    AssertThat(f1, EqualsWithDelta(e_f1, delta));
    AssertThat(p, EqualsWithDelta(e_p, delta));
    AssertThat(r, EqualsWithDelta(e_r, delta));
    AssertThat(avg_p, EqualsWithDelta(e_avg_p, delta));
    AssertThat(ndcg, EqualsWithDelta(e_ndcg, delta));
}
}

go_bandit([]() {

    describe("[ir-eval] retrieval metrics", []() {

        it("should give results on [0, 1] for all measures", []() {
            filesystem::remove_all("ceeaus");
            auto file_cfg = tests::create_config("file");
            auto idx = index::make_index<index::inverted_index>(*file_cfg);
            index::okapi_bm25 ranker;
            index::ir_eval eval{*file_cfg};
            // sanity test bounds
            for (size_t i = 0; i < 5; ++i) {
                auto path = idx->doc_path(doc_id{i});
                corpus::document query{doc_id{0}};
                query.content(filesystem::file_text(path));

                auto ranking = ranker.score(*idx, query);
                auto f1 = eval.f1(ranking, query_id{i});
                auto p = eval.precision(ranking, query_id{i});
                auto r = eval.recall(ranking, query_id{i});
                auto avg_p = eval.avg_p(ranking, query_id{i});
                auto ndcg = eval.ndcg(ranking, query_id{i});
                AssertThat(
                    f1,
                    Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
                AssertThat(
                    p, Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
                AssertThat(
                    r, Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
                AssertThat(
                    avg_p,
                    Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
                AssertThat(
                    ndcg,
                    Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
            }

            AssertThat(eval.map(),
                       Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
            AssertThat(eval.gmap(),
                       Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));

            // geometric mean of numbers is always <= arithmetic mean
            AssertThat(eval.map(), Is().GreaterThanOrEqualTo(eval.gmap()));
        });

        it("should compute correct eval measures", []() {

            auto file_cfg = tests::create_config("file");
            index::ir_eval eval{*file_cfg};
            const double delta = 0.000001;
            AssertThat(eval.map(), EqualsWithDelta(0.0, delta));
            AssertThat(eval.gmap(), EqualsWithDelta(0.0, delta));

            // make some fake results based on the loaded qrels file
            std::vector<index::search_result> results;
            query_id qid{0};
            auto idcg_5 = 1.0 + 1.0 / std::log2(3.0) + 1.0 / std::log2(4.0)
                          + 1.0 / std::log2(5.0) + 1.0 / std::log2(6.0);
            auto idcg = idcg_5 + 1.0 / std::log2(7.0) + 1.0 / std::log2(8.0)
                        + 1.0 / std::log2(9.0) + 1.0 / std::log2(10.0)
                        + 1.0 / std::log2(11.0);

            results.emplace_back(doc_id{0}, 1.0); // relevant
            check_query(eval, results, qid, 0.2 / 1.1, 1, 0.1, 0.1, 1.0 / idcg);
            check_query(eval, results, qid, 0.2 / 1.1, 1, 0.1, 0.2,
                        1.0 / idcg_5, 5);

            results.emplace_back(doc_id{2}, 0.9); // not relevant
            check_query(eval, results, qid, 0.1 / 0.6, 0.5, 0.1, 0.1,
                        1.0 / idcg);
            check_query(eval, results, qid, 0.1 / 0.6, 0.5, 0.1, 0.2,
                        1.0 / idcg_5, 5);

            results.emplace_back(doc_id{1}, 0.8); // relevant
            check_query(eval, results, qid,
                        (2.0 * (2.0 / 3.0) * 0.2) / (2.0 / 3.0 + 0.2),
                        2.0 / 3.0, 0.2, 1.0 / 6.0, 1.5 / idcg);
            check_query(eval, results, qid,
                        (2.0 * (2.0 / 3.0) * 0.2) / (2.0 / 3.0 + 0.2),
                        2.0 / 3.0, 0.2, 1.0 / 3.0, 1.5 / idcg_5, 5);

            results.emplace_back(doc_id{30}, 0.8);  // relevant
            results.emplace_back(doc_id{6}, 0.7);   // relevant
            results.emplace_back(doc_id{43}, 0.6);  // relevant
            results.emplace_back(doc_id{24}, 0.5);  // relevant
            results.emplace_back(doc_id{34}, 0.4);  // relevant
            results.emplace_back(doc_id{35}, 0.3);  // relevant
            results.emplace_back(doc_id{38}, 0.2);  // relevant
            results.emplace_back(doc_id{754}, 0.1); // relevant
            auto avg_p_5
                = (1.0 + 2.0 / 3.0 + 3.0 / 4.0 + 4.0 / 5.0 + 5.0 / 6.0) / 5.0;
            auto avg_p = (1.0 + 2.0 / 3.0 + 3.0 / 4.0 + 4.0 / 5.0 + 5.0 / 6.0
                          + 6.0 / 7.0 + 7.0 / 8.0 + 8.0 / 9.0 + 9.0 / 10.0
                          + 10.0 / 11.0)
                         / 10.0;
            auto dcg_5 = 1.0 + 1.0 / std::log2(4.0) + 1.0 / std::log2(5.0)
                         + 1.0 / std::log2(6.0); // 4 terms, 1 zero term
            auto dcg = dcg_5 + 1.0 / std::log2(7.0) + 1.0 / std::log2(8.0)
                       + 1.0 / std::log2(9.0) + 1.0 / std::log2(10.0)
                       + 1.0 / std::log2(11.0) + 1.0 / std::log2(12.0);
            check_query(eval, results, qid,
                        (2.0 * (10.0 / 11.0)) / ((10.0 / 11.0) + 1.0),
                        10.0 / 11.0, 1.0, avg_p, dcg / idcg);
            check_query(eval, results, qid,
                        (2.0 * (4.0 / 5.0) * 0.4) / ((4.0 / 5.0) + 0.4),
                        4.0 / 5.0, 0.4, avg_p_5, dcg_5 / idcg_5, 5);

            results.erase(results.begin() + 1); // remove non-relevant result
            check_query(eval, results, qid, 1.0, 1.0, 1.0, 1.0, 1.0);
            // recall is still not perfect @5
            check_query(eval, results, qid, 1.0 / 1.5, 1.0, 0.5, 1.0, 1.0, 5);

            // add result with zero AP
            results.clear();
            results.emplace_back(doc_id{2}, 0.9); // not relevant
            avg_p = eval.avg_p(results, qid, 1000);
            AssertThat(avg_p, EqualsWithDelta(0.0, delta));
            AssertThat(eval.map(),
                       Is().GreaterThanOrEqualTo(0).And().LessThanOrEqualTo(1));
            AssertThat(eval.gmap(), EqualsWithDelta(0.0, delta));

            filesystem::remove_all("ceeaus");
        });
    });

    // the magic numbers here are validated with an R implementation
    describe("[ir-eval] rank correlation metrics", []() {
        const double delta = 0.000001;

        it("should throw on nonequal list sizes", []() {
            std::vector<double> rank_x = {1, 2, 3};
            std::vector<double> rank_y = {1, 2, 3, 4};
            auto bad_func = [&]() {
                index::rank_correlation corr{rank_x, rank_y};
            };
            AssertThrows(index::rank_correlation_exception, bad_func());
        });

        it("should calculate Tau A with perfect score", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5};
            std::vector<double> rank_y = {1, 2, 3, 4, 5};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_a(), EqualsWithDelta(1.0, delta));
        });

        it("should calculate Tau A with inverse correlation", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5};
            std::vector<double> rank_y = {5, 4, 3, 2, 1};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_a(), EqualsWithDelta(-1.0, delta));
        });

        it("should calculate Tau A with real score", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {3, 4, 1, 5, 6, 7, 8, 2};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_a(), EqualsWithDelta(0.4285715, delta));
        });

        it("should calculate Tau A with zero score", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {1, 8, 7, 2, 5, 3, 6, 4};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_a(), EqualsWithDelta(0.0, delta));
        });

        it("should calculate Tau B with no ties", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {3, 4, 1, 5, 6, 7, 8, 2};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_a(), EqualsWithDelta(0.4285715, delta));
        });

        it("should calculate Tau B with ties", [&]() {
            std::vector<double> rank_x = {1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<double> rank_y = {1, 2, 3, 4, 5, 6, 7, 8};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_b(), EqualsWithDelta(0.9636242, delta));
        });

        it("should calculate Tau B with ties again", [&]() {
            std::vector<double> rank_x = {1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<double> rank_y = {1, 2, 3, 3, 4, 4, 4, 5};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_b(), EqualsWithDelta(0.9207368, delta));
        });

        it("should calculate NDPM with zero score", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 3, 4, 5, 6, 7};
            std::vector<double> rank_y = {1, 1, 2, 2, 3, 4, 5, 6};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.ndpm(), EqualsWithDelta(0.0, delta));
        });

        it("should calculate NDPM with a real score", [&]() {
            // this is example 3 in
            // the NDPM paper
            std::vector<double> rank_x = {1, 2, 3, 2, 1};
            std::vector<double> rank_y = {1, 1, 2, 3, 3};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.ndpm(), EqualsWithDelta(8.0 / 16.0, delta));
        });

        it("should calculate correct comparative NDPM and Tau B scores", [&]() {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6};
            std::vector<double> rank_y = {1, 1, 2, 2, 3, 4};
            index::rank_correlation corr{rank_x, rank_y};
            AssertThat(corr.tau_b(), EqualsWithDelta(0.9309493, delta));
            AssertThat(corr.ndpm(), EqualsWithDelta(0.0, delta));
        });
    });
});
