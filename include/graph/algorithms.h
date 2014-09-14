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
template <class UndirectedGraph>
double clustering_coefficient(const UndirectedGraph& graph, node_id id);

/**
 * The neighborhood overlap is the ratio of shared neighbors between src and
 * dest, ranging from 0 (no shared neighbors; a local bridge) to 1.
 * @param graph
 * @param src
 * @param dest
 * @return the neighborhood overlap between the two given nodes
 */
template <class UndirectedGraph>
double neighborhood_overlap(const UndirectedGraph& graph, node_id src,
                            node_id dest);

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
