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
void test_rank(Ranker& r, Index& idx, const std::string& encoding)
{
    for (size_t i = 0; i < idx.num_docs(); ++i)
    {
        auto d_id = idx.docs()[i];
        corpus::document query{idx.doc_path(d_id), doc_id{i}};
        query.encoding(encoding);

        auto ranking = r.score(idx, query);
        ASSERT_EQUAL(ranking.size(), 10ul); // default is 10 docs

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
    system("rm -rf ceeaus-inv");
    auto idx = index::make_index<index::inverted_index, caching::splay_cache>(
        "test-config.toml", uint32_t{10000});

    auto config = cpptoml::parse_file("test-config.toml");
    std::string encoding = "utf-8";
    if (auto enc = config.get_as<std::string>("encoding"))
        encoding = *enc;

    int num_failed = 0;
    num_failed += testing::run_test("ranker-absolute-discount", [&]()
    {
        index::absolute_discount r;
        test_rank(r, *idx, encoding);
    });

    num_failed += testing::run_test("ranker-dirichlet-prior", [&]()
    {
        index::dirichlet_prior r;
        test_rank(r, *idx, encoding);
    });

    num_failed += testing::run_test("ranker-jelinek-mercer", [&]()
    {
        index::jelinek_mercer r;
        test_rank(r, *idx, encoding);
    });

    num_failed += testing::run_test("ranker-okapi-bm25", [&]()
    {
        index::okapi_bm25 r;
        test_rank(r, *idx, encoding);
    });

    num_failed += testing::run_test("ranker-pivoted-length", [&]()
    {
        index::pivoted_length r;
        test_rank(r, *idx, encoding);
    });

    idx = nullptr;

    system("rm -rf ceeaus-inv test-config.toml");
    return num_failed;
}
}
}
