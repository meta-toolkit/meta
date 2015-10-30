/**
 * @file ir_eval_test.cpp
 * @author Sean Massung
 */

#include "test/ir_eval_test.h"
#include "index/eval/rank_correlation.h"
#include "index/ranker/ranker.h"
#include "corpus/document.h"

namespace meta
{
namespace testing
{

void check_query(index::ir_eval& eval,
                 const std::vector<index::search_result>& ranking, query_id qid,
                 double e_f1, double e_p, double e_r, double e_avg_p,
                 double e_ndcg,
                 uint64_t num_docs = std::numeric_limits<uint64_t>::max())
{
    auto f1 = eval.f1(ranking, qid, num_docs);
    auto p = eval.precision(ranking, qid, num_docs);
    auto r = eval.recall(ranking, qid, num_docs);
    auto avg_p = eval.avg_p(ranking, qid, num_docs);
    auto ndcg = eval.ndcg(ranking, qid, num_docs);
    ASSERT_APPROX_EQUAL(f1, e_f1);
    ASSERT_APPROX_EQUAL(p, e_p);
    ASSERT_APPROX_EQUAL(r, e_r);
    ASSERT_APPROX_EQUAL(avg_p, e_avg_p);
    ASSERT_APPROX_EQUAL(ndcg, e_ndcg);
}

int ir_eval_bounds()
{
    return testing::run_test(
        "ir-eval-bounds", [&]()
        {
            filesystem::remove_all("ceeaus-inv");
            auto file_cfg = create_config("file");
            auto idx = index::make_index<index::inverted_index>(*file_cfg);
            index::okapi_bm25 ranker;
            index::ir_eval eval{*file_cfg};
            // sanity test bounds
            for (size_t i = 0; i < 5; ++i)
            {
                auto path = idx->doc_path(doc_id{i});
                corpus::document query{doc_id{0}};
                query.content(filesystem::file_text(path));

                auto ranking = ranker.score(*idx, query);
                auto f1 = eval.f1(ranking, query_id{i});
                auto p = eval.precision(ranking, query_id{i});
                auto r = eval.recall(ranking, query_id{i});
                auto avg_p = eval.avg_p(ranking, query_id{i});
                auto ndcg = eval.ndcg(ranking, query_id{i});
                ASSERT(f1 >= 0 && f1 <= 1);
                ASSERT(p >= 0 && p <= 1);
                ASSERT(r >= 0 && r <= 1);
                ASSERT(avg_p >= 0 && avg_p <= 1);
                ASSERT(ndcg >= 0 && ndcg <= 1);
            }

            auto map = eval.map();
            auto gmap = eval.gmap();
            ASSERT(map >= 0 && map <= 1);
            ASSERT(gmap >= 0 && gmap <= 1);
        });
}

int ir_eval_results()
{
    return testing::run_test(
        "ir-eval-results", [&]()
        {
            auto file_cfg = create_config("file");
            index::ir_eval eval{*file_cfg};
            ASSERT_APPROX_EQUAL(eval.map(), 0.0);
            ASSERT_APPROX_EQUAL(eval.gmap(), 0.0);

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
        });
}

// the magic numbers here are validated with an R implementation
int rank_correlation_tests()
{
    int num_failed = 0;

    num_failed += testing::run_test(
        "rank-correlation-throw-nonequal-size", []()
        {
            std::vector<double> rank_x = {1, 2, 3};
            std::vector<double> rank_y = {1, 2, 3, 4};

            try
            {
                index::rank_correlation corr{rank_x, rank_y};
                FAIL("rank_correlation ctor should have thrown");
            }
            catch (const index::rank_correlation_exception&)
            {
                // pass!
            }
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-a-perfect", []()
        {
            std::vector<double> rank_x = {1, 2, 3, 4, 5};
            std::vector<double> rank_y = {1, 2, 3, 4, 5};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_a(), 1.0);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-a-inverse", []()
        {
            std::vector<double> rank_x = {1, 2, 3, 4, 5};
            std::vector<double> rank_y = {5, 4, 3, 2, 1};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_a(), -1.0);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-a-real", []()
        {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {3, 4, 1, 5, 6, 7, 8, 2};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_a(), 0.4285715);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-a-zero", []()
        {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {1, 8, 7, 2, 5, 3, 6, 4};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_a(), 0.0);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-b-noties", []()
        {
            std::vector<double> rank_x = {1, 2, 3, 4, 5, 6, 7, 8};
            std::vector<double> rank_y = {3, 4, 1, 5, 6, 7, 8, 2};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_a(), 0.4285715);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-b-ties-x", []()
        {
            std::vector<double> rank_x = {1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<double> rank_y = {1, 2, 3, 4, 5, 6, 7, 8};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_b(), 0.9636242);
        });

    num_failed += testing::run_test(
        "rank-correlation-tau-b-ties-both", []()
        {
            std::vector<double> rank_x = {1, 1, 2, 2, 3, 4, 5, 6};
            std::vector<double> rank_y = {1, 2, 3, 3, 4, 4, 4, 5};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.tau_b(), 0.9207368);
        });

    num_failed += testing::run_test(
        "rank-correlation-ndpm-perfect", []
        {
            std::vector<double> rank_x = {1, 2, 3, 3, 4, 5, 6, 7};
            std::vector<double> rank_y = {1, 1, 2, 2, 3, 4, 5, 6};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.ndpm(), 0.0);
        });

    num_failed += testing::run_test(
        "rank-correlation-ndpm-real", []
        {
            // this is example 3 in the NDPM paper
            std::vector<double> rank_x = {1, 2, 3, 2, 1};
            std::vector<double> rank_y = {1, 1, 2, 3, 3};
            index::rank_correlation corr{rank_x, rank_y};

            ASSERT_APPROX_EQUAL(corr.ndpm(), 8.0 / 16.0);
        });


    return num_failed;
}

int ir_eval_tests()
{
    int num_failed = 0;
    num_failed += ir_eval_bounds();
    num_failed += ir_eval_results();
    filesystem::remove_all("ceeaus-inv");
    num_failed += rank_correlation_tests();
    return num_failed;
}
}
}
