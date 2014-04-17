/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <iostream>
#include "graph/directed_graph.h"
#include "graph/dblp_node.h"
#include "io/parser.h"

using namespace meta;

void load(graph::directed_graph<graph::dblp_node>& g, const std::string& prefix)
{
    // load nodes

    io::parser authors{prefix + "authors.txt", "\t\n"};
    while (authors.has_next())
        g.insert(graph::dblp_node{"author", authors.next()});

    node_id venue_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    io::parser venues{prefix + "venues.txt", "\t\n"};
    while (venues.has_next())
        g.insert(graph::dblp_node{"venue", venues.next()});

    node_id paper_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    io::parser papers{prefix + "papers.txt", "\t\n"};
    while (papers.has_next())
    {
        papers.next(); // paper year
        g.insert(graph::dblp_node{"paper", papers.next()});
    }

    // load edges

    node_id source;
    node_id dest;

    std::ifstream paper_paper{prefix + "paper_paper.txt"};
    while (paper_paper >> source >> dest)
        g.add_edge(source + paper_offset, dest + paper_offset); // directed

    std::ifstream paper_author{prefix + "paper_author.txt"};
    while (paper_author >> source >> dest)
    {
        try
        {
            g.add_edge(source + paper_offset, node_id{dest - 1});
            // simulate undirected
            g.add_edge(node_id{dest - 1}, source + paper_offset);
        }
        catch (graph::directed_graph_exception& e)
        {
            // it seems there is one duplicate edge in the DBLP data
            std::cout << " > Warning: attempted to add duplicate edge: "
                      << source << " " << dest << std::endl;
        }
    }

    std::ifstream paper_venue{prefix + "paper_venue.txt"};
    while (paper_venue >> source >> dest)
    {
        g.add_edge(source + paper_offset, dest + venue_offset);
        // simulate undirected
        g.add_edge(dest + venue_offset, source + paper_offset);
    }
}

void stats(graph::directed_graph<graph::dblp_node>& g)
{
    std::cout << g.size() << " total nodes" << std::endl;
    std::unordered_map<std::string, uint64_t> counts;
    std::unordered_map<std::string, double> degrees;

    double out_degree = 0.0;
    for (node_id id{0}; id < g.size(); ++id)
    {
        auto node = g.node(id);
        //  std::cout << "Node id " << id << " type: " << node.type
        //            << " name: " << node.name << std::endl;
        ++counts[node.type];
        auto out = g.adjacent(id).size();
        degrees[node.type] += out;
        out_degree += out;
    }

    for (auto& c : counts)
    {
        std::cout << c.first << ": " << c.second << std::endl;
        std::cout << " avg out degree to all types: "
                  << degrees[c.first] / c.second << std::endl;
    }

    std::cout << "Average overall out-degree to all types: "
              << out_degree / g.size() << std::endl;
}

int main()
{
    graph::directed_graph<graph::dblp_node> g;
    std::string prefix = "/home/sean/projects/meta-data/dblp/"; // testing
    load(g, prefix);
    stats(g);
}
