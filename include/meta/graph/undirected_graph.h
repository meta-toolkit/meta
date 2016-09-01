/**
 * @file undirected_graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UNDIRECTED_GRAPH_H_
#define META_UNDIRECTED_GRAPH_H_

#include <cstddef>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "meta/config.h"
#include "meta/graph/default_edge.h"
#include "meta/graph/default_node.h"
#include "meta/graph/graph.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

namespace meta
{
namespace graph
{
/**
 * A simple class to represent a directed graph in memory.
 */
template <class Node = default_node, class Edge = default_edge>
class undirected_graph : public graph<Node, Edge>
{
  public:
    using adjacency_list = typename graph<Node, Edge>::adjacency_list;
    using vec_t = std::vector<std::pair<Node, adjacency_list>>;
    using graph<Node, Edge>::nodes_;
    using graph<Node, Edge>::num_edges_;
    using graph<Node, Edge>::size;
    using graph<Node, Edge>::add_edge;

    /**
     * @param id The node id to get adjacent nodes to
     * @return the connected edges and node_ids to the given node
     */
    const adjacency_list& adjacent(node_id id) const;

    /**
     * @param node The new object to add into the graph
     * @return the id of the inserted node
     */
    virtual node_id insert(Node node) override;

    /**
     * @param edge
     * @param source
     * @param dest
     */
    virtual void add_edge(Edge edge, node_id source, node_id dest) override;

#include "meta/graph/node_iterator.h"
    typedef node_iterator<typename vec_t::iterator> iterator;
    typedef node_iterator<typename vec_t::const_iterator> const_iterator;

    /**
     * @return an iterator to the beginning ("first" node) of this graph
     */
    iterator begin()
    {
        return {nodes_.begin()};
    }

    const_iterator begin() const
    {
        return {nodes_.cbegin()};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    iterator end()
    {
        return {nodes_.end()};
    }

    const_iterator end() const
    {
        return {nodes_.cend()};
    }

#include "meta/graph/edge_iterator.h"
    typedef edge_iterator<typename vec_t::iterator> e_iterator;
    typedef edge_iterator<typename vec_t::const_iterator> const_e_iterator;

    /**
     * @return an iterator to the beginning ("first" edge) of this graph
     */
    e_iterator edges_begin()
    {
        return {this, nodes_.begin(), nodes_.end()};
    }

    const_e_iterator edges_begin() const
    {
        return {this, nodes_.cbegin(), nodes_.cend()};
    }

    /**
     * @return an iterator that represents one past the last node of this graph
     */
    e_iterator edges_end()
    {
        return {this, nodes_.end(), nodes_.end()};
    }

    const_e_iterator edges_end() const
    {
        return {this, nodes_.cend(), nodes_.cend()};
    }

    /**
     * @param filename The file that contains a list of edges in the form "v1
     * v2"
     * @param display_errors Whether to display errors found in the file
     * @return the newly-created graph
     */
    static undirected_graph<Node, Edge> load(const std::string& filename,
                                             bool display_errors = false);
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

#include "meta/graph/undirected_graph.tcc"
#endif
