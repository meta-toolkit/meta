/**
 * @file ir_eval_test.cpp
 * @author Sean Massung
 */

#include "test/ir_eval_test.h"
#include "corpus/document.h"

namespace meta
{
namespace testing
{

int ir_eval_tests()
{
    system("rm -rf ceeaus-inv");
    create_config("file");
    auto idx = index::make_index<index::inverted_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});
    index::okapi_bm25 ranker;
    index::ir_eval eval{"test-config.toml"};

    int num_failed = 0;

    num_failed += testing::run_test("ir-eval", [&]()
    {
        for (size_t i = 0; i < 5; ++i)
        {
            corpus::document query{idx->doc_path(doc_id{i}), doc_id{0}};
            auto ranking = ranker.score(*idx, query);
            double f1 = eval.f1(ranking, query_id{i});
            double p = eval.precision(ranking, query_id{i});
            double r = eval.recall(ranking, query_id{i});
            ASSERT(f1 >= 0 && f1 <= 1);
            ASSERT(p >= 0 && p <= 1);
            ASSERT(r >= 0 && r <= 1);
        }
    });

    system("rm -rf ceeaus-inv test-config.toml");
    return num_failed;
}
}
}
