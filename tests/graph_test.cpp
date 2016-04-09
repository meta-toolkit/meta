/**
 * @file graph_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "meta/graph/algorithms/algorithms.h"
#include "meta/graph/directed_graph.h"
#include "meta/graph/undirected_graph.h"

using namespace bandit;
using namespace meta;

namespace {

template <class Graph>
void check_sizes(const Graph& g, uint64_t num_nodes, uint64_t num_edges) {
    AssertThat(g.size(), Equals(num_nodes));
    AssertThat(g.num_edges(), Equals(num_edges));

    uint64_t count_nodes = 0;
    for (auto& n : g) {
        (void)n; // unused variable warning
        ++count_nodes;
    }
    AssertThat(count_nodes, Equals(num_nodes));

    uint64_t count_edges = 0;
    for (auto it = g.edges_begin(); it != g.edges_end(); ++it)
        ++count_edges;
    AssertThat(count_edges, Equals(num_edges));
}
}

go_bandit([]() {
    using namespace graph;
    const double delta = 0.000001;

    describe("[graph] undirected_graph", [&]() {

        it("should be constructed empty", [&]() {
            undirected_graph<> g;
            check_sizes(g, 0, 0);
        });

        it("should allow node and vertex updates and algorithm calculations",
           [&]() {
               undirected_graph<> g;
               node_id a = g.insert(default_node{"A"});
               node_id b = g.insert(default_node{"B"});
               node_id c = g.insert(default_node{"C"});
               node_id d = g.insert(default_node{"D"});
               check_sizes(g, 4, 0);
               AssertThat(algorithms::clustering_coefficient(g, a),
                          EqualsWithDelta(0.0, delta));

               g.add_edge(a, b);
               g.add_edge(a, c);
               g.add_edge(a, d);
               check_sizes(g, 4, 3);
               AssertThat(g.adjacent(a).size(), Equals(3ul));
               AssertThat(g.adjacent(b).size(), Equals(1ul));
               AssertThat(g.adjacent(c).size(), Equals(1ul));
               AssertThat(g.adjacent(d).size(), Equals(1ul));
               AssertThat(algorithms::clustering_coefficient(g, a),
                          EqualsWithDelta(0.0, delta));
               AssertThat(algorithms::neighborhood_overlap(g, a, b),
                          EqualsWithDelta(0.0, delta));

               g.add_edge(c, d);
               AssertThat(g.adjacent(c).size(), Equals(2ul));
               AssertThat(g.adjacent(d).size(), Equals(2ul));
               check_sizes(g, 4, 4);
               AssertThat(algorithms::clustering_coefficient(g, a),
                          EqualsWithDelta(1.0 / 3, delta));
               AssertThat(algorithms::neighborhood_overlap(g, a, c),
                          EqualsWithDelta(0.5, delta));
               AssertThat(algorithms::neighborhood_overlap(g, d, c),
                          EqualsWithDelta(1.0, delta));

               g.add_edge(b, c);
               AssertThat(algorithms::neighborhood_overlap(g, b, c),
                          EqualsWithDelta(0.5, delta));
               g.add_edge(b, d);
               check_sizes(g, 4, 6);
               AssertThat(algorithms::clustering_coefficient(g, a),
                          EqualsWithDelta(1.0, delta));
               AssertThat(algorithms::neighborhood_overlap(g, b, c),
                          EqualsWithDelta(1.0, delta));
           });
    });

    describe("[graph] directed_graph", [&]() {

        it("should be constructed empty", [&]() {
            directed_graph<> g;
            check_sizes(g, 0, 0);
        });

        it("should allow programmatic node and vertex updates", [&]() {
            directed_graph<> g;
            node_id a = g.insert(default_node{"A"});
            node_id b = g.insert(default_node{"B"});
            node_id c = g.insert(default_node{"C"});
            node_id d = g.insert(default_node{"D"});
            check_sizes(g, 4, 0);

            g.add_edge(a, b);
            g.add_edge(a, c);
            g.add_edge(a, d);
            check_sizes(g, 4, 3);
            AssertThat(g.adjacent(a).size(), Equals(3ul));
            AssertThat(g.adjacent(b).size(), Equals(0ul));
            AssertThat(g.adjacent(c).size(), Equals(0ul));
            AssertThat(g.adjacent(d).size(), Equals(0ul));
            AssertThat(g.incoming(a).size(), Equals(0ul));
            AssertThat(g.incoming(b).size(), Equals(1ul));
            AssertThat(g.incoming(c).size(), Equals(1ul));
            AssertThat(g.incoming(d).size(), Equals(1ul));

            g.add_edge(c, d);
            check_sizes(g, 4, 4);
            AssertThat(g.adjacent(c).size(), Equals(1ul));
            AssertThat(g.adjacent(d).size(), Equals(0ul));
            AssertThat(g.incoming(d).size(), Equals(2ul));

            g.add_edge(d, c); // directed, so a different edge than (c, d)
            check_sizes(g, 4, 5);
        });
    });

    describe("[graph] betweenness centrality", [&]() {

        it("should produce the correct results", [&]() {
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
            AssertThat(scores[0].second, EqualsWithDelta(8.0, delta));
            AssertThat(scores[0].first, Equals(node_id{2}));
            AssertThat(scores[1].second, EqualsWithDelta(6.0, delta));
            AssertThat(scores[2].second, EqualsWithDelta(6.0, delta));
            AssertThat(scores[3].second, EqualsWithDelta(0.0, delta));
            AssertThat(scores[4].second, EqualsWithDelta(0.0, delta));
        });

        it("should produce the correct results on another graph", [&]() {
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
            AssertThat(scores[0].second, EqualsWithDelta(18.0, delta));
            AssertThat(scores[0].first, Equals(node_id{3}));
            AssertThat(scores[1].second, EqualsWithDelta(16.0, delta));
            AssertThat(scores[2].second, EqualsWithDelta(16.0, delta));
            AssertThat(scores[3].second, EqualsWithDelta(0.0, delta));
            AssertThat(scores[4].second, EqualsWithDelta(0.0, delta));
            AssertThat(scores[5].second, EqualsWithDelta(0.0, delta));
            AssertThat(scores[6].second, EqualsWithDelta(0.0, delta));
        });
    });
});
