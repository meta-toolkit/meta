/**
 * @file wiki_page_rank.cpp
 * @author Sean Massung
 *
 * Demo for PageRank and Personalized PageRank.
 * For input files and format, @see http://haselgrove.id.au/wikipedia.htm
 */

#include "cpptoml.h"
#include "meta/graph/algorithms/algorithms.h"
#include "meta/logging/logger.h"
#include "meta/io/filesystem.h"

using namespace meta;

template <class DirectedGraph, class ResultList>
void print_results(const DirectedGraph& g, const ResultList& res,
                   uint64_t top_k)
{
    for (uint64_t idx = 0; idx < top_k && idx < g.size(); ++idx)
    {
        std::cout << " " << (idx + 1) << ". " << g.node(res[idx].first).label
                  << " " << res[idx].second << std::endl;
    }
}

/**
 * Parses the Wikipedia links files and creates a directed graph with nodes
 * labeled as Wikipedia article titles.
 * For input files and format, @see http://haselgrove.id.au/wikipedia.htm
 */
graph::directed_graph<> create_network(const cpptoml::table& config)
{
    auto titles_path = config.get_as<std::string>("wiki-titles");
    if (!titles_path)
        throw std::runtime_error{"wiki-titles param needed in config"};

    auto links_path = config.get_as<std::string>("wiki-links");
    if (!links_path)
        throw std::runtime_error{"wiki-links param needed in config"};

    auto num_nodes = filesystem::num_lines(*titles_path);
    if (num_nodes == 0)
        throw std::runtime_error{"wiki-titles file was empty"};

    graph::directed_graph<> network;
    printing::progress prog{" > Creating graph ", num_nodes};
    std::string line;
    std::ifstream titles_in{*titles_path};
    while (std::getline(titles_in, line))
        network.insert(graph::default_node{line});

    uint64_t idx = 0;
    std::ifstream links_in{*links_path};
    while (std::getline(links_in, line))
    {
        std::stringstream ss{line};
        std::string node_str;
        ss >> node_str;
        node_str.pop_back();
        auto src = node_id{std::stoul(node_str) - 1};
        while (ss >> node_str)
        {
            auto dest = node_id{std::stoul(node_str) - 1};
            network.add_edge(src, dest);
        }
        prog(++idx);
    }
    prog.end();

    return network;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto network = create_network(*config);
    uint64_t top_k = 25;

    // First, run regular PageRank
    auto ranks = graph::algorithms::page_rank_centrality(network, 0.85, {}, 50);
    print_results(network, ranks, top_k);

    // Some example queries, where the id is the titles line # starting from 0
    const auto centers = {
        node_id{1153141}, // Computer_science
        node_id{679246},  // Bill_Gates
        node_id{5315048}, // University_of_Illinois_at_Urbana-Champaign
        node_id{3975552}, // Pizza
        node_id{623970}   // Beer
    };

    // Then, run a Personalized PageRank simulation for some pages
    for (const auto& center : centers)
    {
        std::cout << "Personalized PageRank for \""
                  << network.node(center).label << "\"" << std::endl;
        stats::multinomial<node_id> dist;
        dist.increment(center, 1);
        auto ranks
            = graph::algorithms::page_rank_centrality(network, 0.85, dist, 50);
        print_results(network, ranks, top_k);
    }
}
