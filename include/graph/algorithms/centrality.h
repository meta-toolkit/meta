/**
 * @file centrality.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_ALGORITHMS_CENTRALITY_H_
#define META_GRAPH_ALGORITHMS_CENTRALITY_H_

#include "graph/undirected_graph.h"
#include "graph/directed_graph.h"

#include <vector>

namespace meta
{
namespace graph
{
namespace algorithms
{
using centrality_result = std::vector<std::pair<node_id, double>>;

/**
 * Find the degree centrality of each node in the graph, which is simply the
 * number of adjacent links.
 * @param g
 * @return a collection of (id, centrality) pairs
 */
template <class Graph>
centrality_result degree_centrality(const Graph& g);

/**
 * Find the betweenness centrality of each node in the graph using the algorithm
 * from Ulrik Brandes, 2001.
 * @see http://www.inf.uni-konstanz.de/algo/publications/b-fabc-01.pdf
 * @param g
 * @return a collection of (id, centrality) pairs
 */
template <class Graph>
centrality_result betweenness_centrality(const Graph& g);
namespace internal
{
/**
 * Helper function for betweenness_centrality.
 */
template <class Graph>
void betweenness_step(const Graph& g, centrality_result& cb, node_id n,
                      std::mutex& calc_mut);
}
}
}
}

#include "graph/algorithms/centrality.tcc"
#endif
