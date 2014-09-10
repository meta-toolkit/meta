/**
 * @file unundirected_graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UNDIRECTED_GRAPH_H_
#define META_UNDIRECTED_GRAPH_H_

#include <stdexcept>
#include <vector>
#include <unordered_set>
#include "meta.h"
#include "util/optional.h"
#include "graph/default_node.h"
#include "graph/default_edge.h"

namespace meta
{
namespace graph
{
/**
 * A (currently) simple class to represent a directed graph in memory.
 */
template <class Node = default_node, class Edge = default_edge>
class undirected_graph
{
  public:
    using adjacency_list = std::vector<std::pair<node_id, Edge>>;

    /**
     * @param id The node id to get adjacent nodes to
     * @return the connected edges and node_ids to the given node
     */
    const adjacency_list& adjacent(node_id id) const;

    /**
     * @param id
     * @return the Node object that the id represents
     */
    Node& node(node_id id);

    /**
     * @param source
     * @param dest
     * @return an optional edge connecting source and dest
     */
    util::optional<Edge> edge(node_id source, node_id dest);

    /**
     * @param node The new object to add into the graph
     * @return the id of the inserted node
     */
    node_id insert(Node node);

    /**
     * @param edge
     * @param source
     * @param dest
     */
    void add_edge(const Edge& edge, node_id source, node_id dest);

    /**
     * Adds a default edge between the two nodes.
     * @param source
     * @param dest
     */
    void add_edge(node_id source, node_id dest);

    /**
     * @return the size of this graph (number of nodes), which is the
     * range for a valid node_id
     */
    uint64_t size() const;

    /**
     * @return the number of edges in the graph
     */
    uint64_t num_edges() const;

    #include "node_iterator.h"
    #include "edge_iterator.h"

    /**
     * @return an iterator to the beginning ("first" node) of this graph
     */
    node_iterator begin()
    {
        return {this, node_id{0}};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    node_iterator end()
    {
        return {this, node_id{nodes_.size()}};
    }

    /**
     * @return an iterator to the beginning ("first" edge) of this graph
     */
    edge_iterator edges_begin()
    {
        return {this, node_id{0}, false};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    edge_iterator edges_end()
    {
        return {this, node_id{nodes_.size()}, true};
    }

  private:
    /**
     * Each Node object is indexed by its id.
     */
    std::vector<std::pair<Node, adjacency_list>> nodes_;

    uint64_t num_edges_ = 0;
};

/**
 * Basic exception for undirected_graph interactions.
 */
class undirected_graph_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "graph/undirected_graph.tcc"
#endif
