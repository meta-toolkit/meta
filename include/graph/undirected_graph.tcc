/**
 * @file unundirected_graph.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace graph
{

template <class Node, class Edge>
auto undirected_graph<Node, Edge>::adjacent(node_id id) const -> const
    adjacency_list &
{
    if (id >= size())
        throw undirected_graph_exception{"node_id out of range"};

    return nodes_.at(id).second;
}

template <class Node, class Edge>
Node& undirected_graph<Node, Edge>::node(node_id id)
{
    if (id >= size())
        throw undirected_graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
typename util::optional<Edge> undirected_graph<Node, Edge>::edge(node_id source,
                                                                 node_id dest)
{
    if (source >= size() || dest >= size())
        throw undirected_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        return {it->second};

    return {util::nullopt};
}

template <class Node, class Edge>
node_id undirected_graph<Node, Edge>::insert(Node node)
{
    node.id = nodes_.size();
    nodes_.emplace_back(node, adjacency_list{});
    return node.id;
}

template <class Node, class Edge>
void undirected_graph<Node, Edge>::add_edge(const Edge& edge, node_id source,
                                            node_id dest)
{
    if (source >= size() || dest >= size())
        throw undirected_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        throw undirected_graph_exception{"attempted to add existing edge"};

    // add connections to both adjacency lists
    list.emplace_back(dest, edge);
    nodes_[dest].second.emplace_back(source, edge);
}

template <class Node, class Edge>
void undirected_graph<Node, Edge>::add_edge(node_id source, node_id dest)
{
    add_edge(Edge{}, source, dest);
}

template <class Node, class Edge>
uint64_t undirected_graph<Node, Edge>::size() const
{
    return nodes_.size();
}
}
}
