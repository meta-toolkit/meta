/**
 * @file graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DIRECTED_GRAPH_H_
#define META_DIRECTED_GRAPH_H_

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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
class directed_graph
{
  public:
    using adjacency_list = std::unordered_map<node_id, Edge>;

    /**
     * @param id
     * @return the adjacent edges and node_ids to the given node
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
    node_id insert(const Node& node);

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
     * @param id The node id to get incoming nodes to
     * @return a collection of node_ids that are incoming to the parameter node
     */
    std::unordered_set<node_id> incoming(node_id id) const;

    /**
     * @return the size of this graph (number of nodes), which is the
     * range for a valid node_id
     */
    uint64_t size() const;

    /**
     * @param source
     * @param dest
     * @return whether the given node is visited
     */
    bool edge_visited(node_id source, node_id dest) const;

    /**
     * @param source
     * @param dest
     * @param v
     */
    void edge_visited(node_id source, node_id dest, bool v);

    /**
     * Marks all nodes as not visited.
     */
    void reset_edge_visited();

    /**
     * @param id
     * @return whether the given node is visited
     */
    bool visited(node_id id) const;

    /**
     * Sets the visited status of id to v
     * @param id
     * @param v
     */
    void visited(node_id id, bool v);

    /**
     * Marks all nodes as not visited.
     */
    void reset_visited();

  private:
    /**
     * Each Node object is indexed by its id. This keeps track of outgoing
     * edges.
     */
    std::vector<std::pair<Node, adjacency_list>> nodes_;

    /**
     * Each Node object is indexed by its id. This structure keeps track of
     * incoming nodes to a specific node_id.
     */
    std::vector<std::unordered_set<node_id>> incoming_;

    /// Keeps track of whether nodes have been visited for the user.
    std::unordered_set<node_id> visited_;

    /// Keeps track of whether edges have been visited for the user.
    std::unordered_set<std::string> edge_visited_;
};

/**
 * Basic exception for directed_graph interactions.
 */
class directed_graph_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "graph/directed_graph.tcc"
#endif
