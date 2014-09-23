/**
 * @file algorithms.h
 * @author Sean Massung
 */

#ifndef META_GRAPH_ALGORITHMS_H_
#define META_GRAPH_ALGORITHMS_H_

#include "graph/undirected_graph.h"

namespace meta
{
namespace graph
{
namespace algorithms
{
/**
 * The clustering coefficient of a given node n measures how close to a
 * clique n's neighborhood is. It is a ratio of the number of connections
 * between n's neighbors and the total possible number of connections
 * between n's neighbors.
 * @param graph
 * @param id
 * @return the clustering coefficient of the given node
 */
template <class Graph>
double clustering_coefficient(const Graph& graph, node_id id);

/**
 * Computes the clustering coefficient of the entire graph as the average
 * clustering coefficient of each node.
 * @return the graph's clustering coefficient
 */
template <class Graph>
double clustering_coefficient(const Graph& graph);

/**
 * The neighborhood overlap is the ratio of shared neighbors between src and
 * dest, ranging from 0 (no shared neighbors; a local bridge) to 1.
 * @param graph
 * @param src
 * @param dest
 * @return the neighborhood overlap between the two given nodes
 */
template <class Graph>
double neighborhood_overlap(const Graph& graph, node_id src, node_id dest);

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
 * Exception for errors in graph algorithms.
 */
class graph_algorithm_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
}

#include "algorithms.tcc"
#endif
