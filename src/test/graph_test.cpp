/**
 * @file undirected_graph_test.cpp
 * @author Sean Massung
 */

#include "test/graph_test.h"
#include "graph/algorithms/algorithms.h"

namespace meta
{
namespace testing
{

template <class Graph>
void check_sizes(const Graph& g, uint64_t num_nodes, uint64_t num_edges)
{
    ASSERT_EQUAL(g.size(), num_nodes);
    ASSERT_EQUAL(g.num_edges(), num_edges);

    uint64_t count_nodes = 0;
    for (auto& n : g)
    {
        (void)n; // unused variable warning
        ++count_nodes;
    }

    ASSERT_EQUAL(num_nodes, count_nodes);

    uint64_t count_edges = 0;
    for (auto it = g.edges_begin(); it != g.edges_end(); ++it)
        ++count_edges;

    ASSERT_EQUAL(num_edges, count_edges);
}

int test_undirected()
{
    return testing::run_test("undirected-graph", [&]()
    {
        using namespace graph;
        undirected_graph<> g;
        check_sizes(g, 0, 0);

        node_id a = g.insert(default_node{"A"});
        node_id b = g.insert(default_node{"B"});
        node_id c = g.insert(default_node{"C"});
        node_id d = g.insert(default_node{"D"});
        check_sizes(g, 4, 0);
        ASSERT_APPROX_EQUAL(algorithms::clustering_coefficient(g, a), 0.0);

        g.add_edge(a, b);
        g.add_edge(a, c);
        g.add_edge(a, d);
        check_sizes(g, 4, 3);
        ASSERT_APPROX_EQUAL(algorithms::clustering_coefficient(g, a), 0.0);
        ASSERT_APPROX_EQUAL(algorithms::neighborhood_overlap(g, a, b), 0.0);

        g.add_edge(c, d);
        check_sizes(g, 4, 4);
        ASSERT_APPROX_EQUAL(algorithms::clustering_coefficient(g, a), 1.0 / 3);
        ASSERT_APPROX_EQUAL(algorithms::neighborhood_overlap(g, a, c), 0.5);
        ASSERT_APPROX_EQUAL(algorithms::neighborhood_overlap(g, d, c), 1.0);

        g.add_edge(b, c);
        ASSERT_APPROX_EQUAL(algorithms::neighborhood_overlap(g, b, c), 0.5);
        g.add_edge(b, d);
        check_sizes(g, 4, 6);
        ASSERT_APPROX_EQUAL(algorithms::clustering_coefficient(g, a), 1.0);
        ASSERT_APPROX_EQUAL(algorithms::neighborhood_overlap(g, b, c), 1.0);
    });
}

int test_directed()
{
    return testing::run_test("directed-graph", [&]()
    {
        using namespace graph;
        directed_graph<> g;
        check_sizes(g, 0, 0);

        node_id a = g.insert(default_node{"A"});
        node_id b = g.insert(default_node{"B"});
        node_id c = g.insert(default_node{"C"});
        node_id d = g.insert(default_node{"D"});
        check_sizes(g, 4, 0);

        g.add_edge(a, b);
        g.add_edge(a, c);
        g.add_edge(a, d);
        check_sizes(g, 4, 3);

        g.add_edge(c, d);
        check_sizes(g, 4, 4);

        g.add_edge(d, c); // directed, so this is a different edge than (c, d)
        check_sizes(g, 4, 5);
    });
}

int test_betweenness()
{
    int num_failed = 0;

    num_failed += testing::run_test("betweenness", [&]()
    {
        using namespace graph;
        undirected_graph<> g;

        auto a = g.emplace("a");
        auto b = g.emplace("b");
        auto c = g.emplace("c");
        auto d = g.emplace("d");
        auto e = g.emplace("e");
        g.add_edge(a, b);
        g.add_edge(b, c);
        g.add_edge(c, d);
        g.add_edge(d, e);

        auto scores = algorithms::betweenness_centrality(g);
        ASSERT_APPROX_EQUAL(scores[0].second, 8.0);
        ASSERT_EQUAL(scores[0].first, node_id{2});
        ASSERT_APPROX_EQUAL(scores[1].second, 6.0);
        ASSERT_APPROX_EQUAL(scores[2].second, 6.0);
        ASSERT_APPROX_EQUAL(scores[3].second, 0.0);
        ASSERT_APPROX_EQUAL(scores[4].second, 0.0);
    });

    num_failed += testing::run_test("betweenness", [&]()
    {
        using namespace graph;
        undirected_graph<> g;

        auto a = g.emplace("a");
        auto b = g.emplace("b");
        auto c = g.emplace("c");
        auto d = g.emplace("d");
        auto e = g.emplace("e");
        auto f = g.emplace("f");
        auto h = g.emplace("h");
        g.add_edge(a, b);
        g.add_edge(b, c);
        g.add_edge(a, c);
        g.add_edge(c, d);
        g.add_edge(d, e);
        g.add_edge(e, f);
        g.add_edge(e, h);
        g.add_edge(f, h);

        auto scores = algorithms::betweenness_centrality(g);
        ASSERT_APPROX_EQUAL(scores[0].second, 18.0);
        ASSERT_EQUAL(scores[0].first, node_id{3});
        ASSERT_APPROX_EQUAL(scores[1].second, 16.0);
        ASSERT_APPROX_EQUAL(scores[2].second, 16.0);
        ASSERT_APPROX_EQUAL(scores[3].second, 0.0);
        ASSERT_APPROX_EQUAL(scores[4].second, 0.0);
        ASSERT_APPROX_EQUAL(scores[5].second, 0.0);
        ASSERT_APPROX_EQUAL(scores[6].second, 0.0);
    });

    return num_failed;
}

int graph_tests()
{
    int num_failed = 0;
    num_failed += test_undirected();
    num_failed += test_directed();
    num_failed += test_betweenness();
    return num_failed;
}
}
}
