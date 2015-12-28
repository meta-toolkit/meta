/**
 * @file unundirected_graph.tcc
 * @author Sean Massung
 */

#include <unordered_map>
#include <iostream>
#include <fstream>

namespace meta
{
namespace graph
{

template <class Node, class Edge>
auto undirected_graph<Node, Edge>::adjacent(node_id id) const
    -> const adjacency_list &
{
    if (id >= size())
        throw undirected_graph_exception{"node_id out of range"};

    return nodes_.at(id).second;
}

template <class Node, class Edge>
node_id undirected_graph<Node, Edge>::insert(Node node)
{
    node.id = node_id{size()};
    nodes_.emplace_back(node, adjacency_list{});
    return node.id;
}

template <class Node, class Edge>
void undirected_graph<Node, Edge>::add_edge(Edge edge, node_id source,
                                            node_id dest)
{
    if (source == dest)
        throw undirected_graph_exception{"can not create self-loops"};

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

    edge.src = source;
    edge.dest = dest;
    ++num_edges_;

    // add connections to both adjacency lists
    list.emplace_back(dest, edge);
    nodes_[dest].second.emplace_back(source, edge);
}

template <class Node, class Edge>
undirected_graph<Node, Edge>
undirected_graph<Node, Edge>::load(const std::string& filename,
                                   bool display_errors /* = false */)
{
    undirected_graph<Node, Edge> g;

    std::string src;
    std::string dest;
    std::ifstream infile{filename};
    std::unordered_map<std::string, node_id> seen;
    size_t tried = 0;
    size_t errors = 0;
    while (infile >> src >> dest)
    {
        ++tried;

        if (src == dest)
        {
            if (display_errors)
                std::cout << "Found self-loop: " << src << std::endl;
            continue;
        }

        if (seen.find(src) == seen.end())
            seen[src] = g.emplace(src);

        if (seen.find(dest) == seen.end())
            seen[dest] = g.emplace(dest);

        try
        {
            g.add_edge(seen[src], seen[dest]);
        }
        catch (undirected_graph_exception& e)
        {
            if (display_errors)
            {
                ++errors;
                std::cout << e.what() << std::endl;
            }
        }
    }

    if (display_errors)
    {
        std::cout << "Tried to add " << tried << " total edges." << std::endl;
        std::cout << "Got " << errors << " errors." << std::endl;
    }

    return g;
}
}
}
