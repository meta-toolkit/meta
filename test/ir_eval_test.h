/**
 * @file ir_eval_test.h
 * @author Sean Massung
 */

#ifndef _META_IR_EVAL_TEST_H_
#define _META_IR_EVAL_TEST_H_

#include "inverted_index_test.h"
#include "index/eval/ir_eval.h"
#include "index/ranker/okapi_bm25.h"
#include "unit_test.h"

namespace meta
{
namespace testing
{

void ir_eval_tests()
{
    system("/usr/bin/rm -rf ceeaus-inv");
    create_config("file");
    auto idx = index::make_index<index::inverted_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});
    index::okapi_bm25 ranker;
    index::ir_eval eval{"test-config.toml"};

    testing::run_test("ir-eval", [&]()
    {
        for (size_t i = 0; i < 5; ++i)
        {
            corpus::document query{idx.doc_path(i), doc_id{0}};
            auto ranking = ranker.score(idx, query);
            double f1 = eval.f1(ranking, query_id{i});
            double p = eval.precision(ranking, query_id{i});
            double r = eval.recall(ranking, query_id{i});
            ASSERT(f1 >= 0 && f1 <= 1);
            ASSERT(p >= 0 && p <= 1);
            ASSERT(r >= 0 && r <= 1);
        }
    });

    system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
}
}
}

#endif
