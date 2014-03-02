/**
 * @file ranker_test.cpp
 * @author Sean Massung
 */

#include "test/ranker_test.h"
#include "corpus/document.h"

namespace meta
{
namespace testing
{

template <class Ranker, class Index>
void test_rank(Ranker& r, Index& idx)
{
    for (size_t i = 0; i < idx.num_docs(); ++i)
    {
        auto d_id = idx.docs()[i];
        corpus::document query{idx.doc_path(d_id), doc_id{i}};

        auto ranking = r.score(idx, query);
        ASSERT_EQUAL(ranking.size(), 10); // default is 10 docs

        // since we're searching for a document already in the index, the same
        // document should be ranked first, but there are a few duplicate
        // documents......
        if (ranking[0].first != i)
        {
            ASSERT_EQUAL(ranking[1].first, i);
            ASSERT_APPROX_EQUAL(ranking[0].second, ranking[1].second);
        }
    }
}

int ranker_tests()
{
    create_config("file");
    system("/usr/bin/rm -rf ceeaus-inv");
    auto idx = index::make_index<index::inverted_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});

    int num_failed = 0;
    int timeout = 10; // 10 seconds
    /* TODO why does this not always work?
    num_failed += testing::run_test("ranker-absolute-discount", timeout, [&]()
    {
        index::absolute_discount r;
        test_rank(r, idx);
    });
    */
    num_failed += testing::run_test("ranker-dirichlet-prior", timeout, [&]()
    {
        index::dirichlet_prior r;
        test_rank(r, idx);
    });

    num_failed += testing::run_test("ranker-jelinek-mercer", timeout, [&]()
    {
        index::jelinek_mercer r;
        test_rank(r, idx);
    });

    num_failed += testing::run_test("ranker-okapi-bm25", timeout, [&]()
    {
        index::okapi_bm25 r;
        test_rank(r, idx);
    });

    num_failed += testing::run_test("ranker-pivoted-length", timeout, [&]()
    {
        index::pivoted_length r;
        test_rank(r, idx);
    });

    system("/usr/bin/rm -rf ceeaus-inv test-config.toml");
    testing::report(num_failed);
    return num_failed;
}
}
}
