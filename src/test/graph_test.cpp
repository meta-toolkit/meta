/**
 * @file undirected_graph_test.cpp
 * @author Sean Massung
 */

#include "test/graph_test.h"
#include "graph/algorithms.h"

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
        graph::undirected_graph<> g;
        check_sizes(g, 0, 0);

        node_id a = g.insert(graph::default_node{"A"});
        node_id b = g.insert(graph::default_node{"B"});
        node_id c = g.insert(graph::default_node{"C"});
        node_id d = g.insert(graph::default_node{"D"});
        check_sizes(g, 4, 0);
        ASSERT_APPROX_EQUAL(graph::algorithms::clustering_coefficient(g, a),
                            0.0);

        g.add_edge(a, b);
        g.add_edge(a, c);
        g.add_edge(a, d);
        check_sizes(g, 4, 3);
        ASSERT_APPROX_EQUAL(graph::algorithms::clustering_coefficient(g, a),
                            0.0);

        g.add_edge(c, d);
        check_sizes(g, 4, 4);
        ASSERT_APPROX_EQUAL(graph::algorithms::clustering_coefficient(g, a),
                            1.0 / 3.0);

        g.add_edge(b, c);
        g.add_edge(b, d);
        check_sizes(g, 4, 6);
        ASSERT_APPROX_EQUAL(graph::algorithms::clustering_coefficient(g, a),
                            1.0);
    });
}

int test_directed()
{
    return testing::run_test("directed-graph", [&]()
    {
        graph::directed_graph<> g;
        check_sizes(g, 0, 0);

        node_id a = g.insert(graph::default_node{"A"});
        node_id b = g.insert(graph::default_node{"B"});
        node_id c = g.insert(graph::default_node{"C"});
        node_id d = g.insert(graph::default_node{"D"});
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

int graph_tests()
{
    int num_failed = 0;
    num_failed += test_undirected();
    num_failed += test_directed();
    return num_failed;
}
}
}
