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
 * @param id
 * @return the clustering coefficient of the given node
 */
template <class UndirectedGraph>
double clustering_coefficient(const UndirectedGraph& graph, node_id id);
}
}
}

#include "algorithms.tcc"
#endif
