/**
 * @file path_predict.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_GRAPH_PATH_PREDICT_H_
#define META_GRAPH_PATH_PREDICT_H_

#include <iostream>
#include <vector>
#include <unordered_map>
#include "corpus/document.h"
#include "graph/dblp_node.h"
#include "graph/dblp_loader.h"
#include "graph/directed_graph.h"
#include "graph/algorithm/metapath_measures.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
/**
 * Implementation of the PathPredict algorithm for heterogeneous information
 * networks.
 * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
 * Networks, Sun et. al. 2011.
 */
class path_predict
{
  public:
    using graph_t = directed_graph<dblp_node>;
    using node_pair = std::pair<node_id, node_id>;

    /**
     * @param config_file The toml config file used to create this instance of
     * the PathPredict algorithm
     */
    path_predict(const std::string& config_file);

    /**
     * @return a corpus made from the graph
     */
    std::vector<corpus::document> docs() const;

  private:
    /**
     * @return positive and negative documents representing pairs of
     * potentially-collaborating authors
     */
    void create_docs();

    /**
     * @param one
     * @param two
     * @param g The graph to use to check for coauthors
     * @return whether the two nodes are linked
     */
    bool coauthors(node_id one, node_id two, graph_t& g);

    /**
     * @return a mapping of (node id, node id) -> document of authors that are
     * a certain distance away
     */
    std::unordered_map<node_pair, corpus::document> three_hop_authors();

    /// The graph at time step 0
    graph_t g_before_;

    /// The graph at time step 1
    graph_t g_after_;

    /// The documents representing potential links between nodes
    std::vector<corpus::document> docs_;
};

/**
 * Basic exception for path_predict interactions.
 */
class path_predict_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
}

namespace std
{
template <>
struct hash<std::pair<meta::node_id, meta::node_id>>
{
    size_t operator()(const std::pair<meta::node_id, meta::node_id>& p) const
    {
        return p.first * 100000 + p.second;
    }
};
}

#endif
