/**
 * @file graph-test.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <iostream>
#include <queue>
#include "graph/directed_graph.h"
#include "graph/algorithm/metapath_measures.h"
#include "graph/dblp_node.h"
#include "logging/logger.h"
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

    node_id term_offset{g.size() - 1};

    io::parser terms{prefix + "terms.txt", "\n"};
    while (terms.has_next())
        g.insert(graph::dblp_node{"term", terms.next()});

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

    std::ifstream paper_term{prefix + "paper_term.txt"};
    while (paper_term >> source >> dest)
    {
        g.add_edge(source + paper_offset, dest + term_offset);
        g.add_edge(dest + term_offset, source + paper_offset);
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
        // std::cout << "Node id " << id << " type: " << node.type
        //          << " name: " << node.name << std::endl;
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

void measure(graph::directed_graph<graph::dblp_node>& g,
             const std::vector<std::string>& metapath)
{
    graph::algorithm::metapath_measures<decltype(g)> m{g, metapath};
    using pair_t = std::pair<std::string, double>;
    auto pair_comp = [](const pair_t& a, const pair_t& b)
    { return a.second > b.second; };
    std::priority_queue
        <pair_t, std::vector<pair_t>, decltype(pair_comp)> pq{pair_comp};
    // for (auto& m1 : m.path_count())
    // for (auto& m1 : m.normalized_path_count())
    // for (auto& m1 : m.random_walk())
    for (auto& m1 : m.symmetric_random_walk())
    {
        for (auto& m2 : m1.second)
        {
            auto src = g.node(m1.first).name;
            auto dest = g.node(m2.first).name;
            if (m1.first > m2.first) // remove duplicate (reversed) paths
                std::swap(src, dest);

            // it only really makes sense to sort PathCount
            pq.emplace(src + " <-> " + dest, m2.second);
            if (pq.size() > 25)
                pq.pop();
        }
    }

    std::vector<pair_t> sorted;
    while (!pq.empty())
    {
        sorted.emplace_back(pq.top());
        pq.pop();
    }
    std::reverse(sorted.begin(), sorted.end());

    for (auto& s : sorted)
        std::cout << s.first << " " << s.second << std::endl;
}

int main()
{
    logging::set_cerr_logging();

    graph::directed_graph<graph::dblp_node> g;
    // std::string prefix = "../data/mini-dblp/"; // testing
    std::string prefix = "/home/sean/projects/meta-data/dblp/"; // testing
    load(g, prefix);
    stats(g);
    // measure(g, {"author", "paper", "paper", "paper", "author"});
    // measure(g, {"author", "paper", "paper", "author"});
    // measure(g, {"author", "paper", "author", "paper", "author"});
    // measure(g, {"author", "paper", "venue", "paper", "author"});
    // measure(g, {"author", "paper", "term", "paper", "author"});
    measure(g, {"author", "paper", "author"});
}
