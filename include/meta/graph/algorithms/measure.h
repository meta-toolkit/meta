/**
 * @file measure.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_ALGORITHMS_MEASURE_H_
#define META_GRAPH_ALGORITHMS_MEASURE_H_

#include "meta/graph/undirected_graph.h"
#include "meta/graph/directed_graph.h"

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
}
}
}

#include "meta/graph/algorithms/measure.tcc"
#endif
