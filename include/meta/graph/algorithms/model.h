/**
 * @file model.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_ALGORITHMS_MODEL_H_
#define META_GRAPH_ALGORITHMS_MODEL_H_

#include "meta/graph/undirected_graph.h"
#include "meta/graph/directed_graph.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
/**
 * @param g
 * @param num_nodes The total number of nodes in the graph
 * @param num_edges The total number of edges in the graph
 */
template <class Graph>
void random_graph(Graph& g, uint64_t num_nodes, uint64_t num_edges);

/**
 * @param g
 * @param num_nodes The total number of nodes in the graph
 * @param num_neighbors An even number of neighbor nodes to connect to
 * @param num_random_edges The number of random edges to add to the ring network
 */
template <class Graph>
void watts_strogatz(Graph& g, uint64_t num_nodes, uint64_t num_neighbors,
                    uint64_t num_random_edges);

/**
 * @param g
 * @param num_nodes The number of nodes to add to the graph (the total number
 * of time steps)
 * @param node_edges How many edges to create per node
 * @param attr A function that takes a node_id and returns its attractiveness
 * in [0, 1]; nodes with higher attractiveness are more likely to be connected
 * to. The default attractiveness function makes all nodes have the same
 * attractiveness.
 */
template <class Graph>
void preferential_attachment(Graph& g, uint64_t num_nodes, uint64_t node_edges,
        std::function<double(node_id)> attr = [](node_id) { return 1.0; });
}
}
}

#include "meta/graph/algorithms/model.tcc"

#endif
