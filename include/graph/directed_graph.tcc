/**
 * @file directed_graph.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace graph
{
template <class Node, class Edge>
auto directed_graph
    <Node, Edge>::adjacent(node_id id) const -> const adjacency_list &
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].second;
}

template <class Node, class Edge>
Node& directed_graph<Node, Edge>::node(node_id id)
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
typename util::optional<Edge> directed_graph
    <Node, Edge>::edge(node_id source, node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = list.find(dest);
    if (it != list.end())
        return {it->second};

    return {util::nullopt};
}

template <class Node, class Edge>
node_id directed_graph<Node, Edge>::insert(const Node& node)
{
    nodes_.emplace_back(node, adjacency_list{});
    incoming_.emplace_back(std::unordered_set<node_id>{});
    return node_id{nodes_.size() - 1};
}

template <class Node, class Edge>
void directed_graph
    <Node, Edge>::add_edge(const Edge& edge, node_id source, node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = list.find(dest);
    if (it != list.end())
        throw directed_graph_exception{"attempted to add existing edge"};

    list.emplace(dest, edge);       // add outgoing edge from source to dest
    incoming_[source].insert(dest); // add incoming edge to source
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::add_edge(node_id source, node_id dest)
{
    add_edge(Edge{}, source, dest);
}

template <class Node, class Edge>
std::unordered_set<node_id> directed_graph
    <Node, Edge>::incoming(node_id id) const
{
    if(id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return incoming_.at(id);
}

template <class Node, class Edge>
uint64_t directed_graph<Node, Edge>::size() const
{
    return nodes_.size();
}

template <class Node, class Edge>
bool directed_graph<Node, Edge>::visited(node_id id) const
{
    return visited_.find(id) != visited_.end();
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::visited(node_id id, bool v)
{
    if (v)
        visited_.insert(id);
    else
        visited_.erase(id);
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::reset_visited()
{
    visited_.clear();
}

template <class Node, class Edge>
bool directed_graph
    <Node, Edge>::edge_visited(node_id source, node_id dest) const
{
    auto key = std::to_string(source) + "_" + std::to_string(dest);
    return edge_visited_.find(key) != edge_visited_.end();
}

template <class Node, class Edge>
void directed_graph
    <Node, Edge>::edge_visited(node_id source, node_id dest, bool v)
{
    if (v)
    {
        edge_visited_.insert(std::to_string(source) + "_"
                             + std::to_string(dest));
        if (edge(source, dest))
            edge_visited_.insert(std::to_string(dest) + "_"
                                 + std::to_string(source));
    }
    else
    {
        edge_visited_.erase(std::to_string(source) + "_"
                            + std::to_string(dest));
        edge_visited_.erase(std::to_string(dest) + "_"
                            + std::to_string(source));
    }
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::reset_edge_visited()
{
    edge_visited_.clear();
}
}
}
