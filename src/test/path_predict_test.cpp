/**
 * @file path_predict_test.cpp
 * @author Sean Massung
 */

#include "test/unit_test.h"
#include "test/path_predict_test.h"
#include "graph/dblp_loader.h"
#include "graph/metapath.h"

namespace meta
{
namespace testing
{
int path_predict_tests()
{
    int num_failed = 0;

    using graph_t = graph::directed_graph<graph::dblp_node>;

    graph_t dblp;
    graph::dblp_loader::load(dblp, "../data/mini-dblp/", 2000, 2010);
    graph::metapath path{"author -- paper -- venue -- paper -- author"};
    graph::algorithm::metapath_measures<graph_t> meas{dblp, path};

    node_id jim_id = 0;
    node_id mike_id = 0;

    for(node_id id{0}; id < dblp.size(); ++id)
    {
        if(dblp.node(id).name == "Jim")
            jim_id = id;
        else if(dblp.node(id).name == "Mike")
            mike_id = id;
    }

    num_failed += testing::run_test("APVPA-PC-example", [&]()
    {
        auto pc = meas.path_count();
        ASSERT_APPROX_EQUAL(pc[jim_id][mike_id], 7.0);
        ASSERT_APPROX_EQUAL(pc[mike_id][jim_id], 7.0);
        ASSERT_APPROX_EQUAL(pc[jim_id][jim_id], 7.0);
        ASSERT_APPROX_EQUAL(pc[mike_id][mike_id], 9.0);
    });

    num_failed += testing::run_test("APVPA-NPC-example", [&]()
    {

        auto npc = meas.normalized_path_count();
        ASSERT_APPROX_EQUAL(npc[jim_id][mike_id], 7.0 / 8.0);
        ASSERT_APPROX_EQUAL(npc[mike_id][jim_id], 7.0 / 8.0);
        ASSERT_APPROX_EQUAL(npc[mike_id][mike_id], 1.0);
        ASSERT_APPROX_EQUAL(npc[jim_id][jim_id], 1.0);
    });

    num_failed += testing::run_test("APVPA-RW-example", [&]()
    {
        auto rw = meas.random_walk();
        ASSERT_APPROX_EQUAL(rw[jim_id][mike_id], 1.0 / 2.0);
        ASSERT_APPROX_EQUAL(rw[jim_id][jim_id], 1.0 / 2.0);
        ASSERT_APPROX_EQUAL(rw[mike_id][jim_id], 7.0 / 16.0);
        ASSERT_APPROX_EQUAL(rw[mike_id][mike_id], 9.0 / 16.0);
    });

    num_failed += testing::run_test("APVPA-SRW-example", [&]()
    {
        auto srw = meas.symmetric_random_walk();
        ASSERT_APPROX_EQUAL(srw[jim_id][mike_id], 15.0 / 16.0);
        ASSERT_APPROX_EQUAL(srw[jim_id][jim_id], 1.0);
        ASSERT_APPROX_EQUAL(srw[mike_id][jim_id], 15.0 / 16.0);
        ASSERT_APPROX_EQUAL(srw[mike_id][mike_id], 9.0 / 8.0);
    });

    return num_failed;
}
}
}
