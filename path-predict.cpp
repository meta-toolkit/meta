/**
 * @file path-predict.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include "corpus/document.h"
#include "graph/directed_graph.h"
#include "graph/algorithm/metapath_measures.h"
#include "graph/dblp_node.h"
#include "graph/dblp_loader.h"
#include "logging/logger.h"

using namespace meta;

namespace std
{
template <>
struct hash<std::pair<node_id, node_id>>
{
    size_t operator()(const std::pair<node_id, node_id>& p) const
    {
        return p.first * 100000 + p.second;
    }
};
}

template <class Graph>
std::unordered_map<std::pair<node_id, node_id>, corpus::document>
    three_hop_authors(Graph& g)
{
    using node_pair = std::pair<node_id, node_id>;
    std::unordered_map<node_pair, corpus::document> docs;
    graph::algorithm::metapath_measures<Graph> measures{
        g, graph::metapath{"author -- paper -- author -- paper -- author"}};
    for (auto& srcp : measures.path_count())
    {
        for (auto& destp : srcp.second)
        {
            if (srcp.first < destp.first)
            {
                docs[std::make_pair(srcp.first, destp.first)]
                    = corpus::document{};
            }
        }
    }
    std::cout << "Found " << docs.size() << " three-hop author pairs"
              << std::endl;
    return docs;
}

/**
 * @param feature
 * @param feature_map
 */
uint64_t get_id(const std::string& feature,
                std::unordered_map<std::string, uint64_t>& feature_map)
{
    auto it = feature_map.find(feature);
    if (it == feature_map.end())
    {
        auto next_id = feature_map.size();
        feature_map[feature] = next_id;
        return next_id;
    }

    return it->second;
}

template <class Graph>
bool coauthors(Graph& g, node_id one, node_id two)
{
    graph::algorithm::metapath_measures
        <Graph> measures{g, graph::metapath{"author -- paper -- author"}};
    typename graph::algorithm::metapath_measures<Graph>::measure_result res;
    measures.bfs_match(one, one, res, 0);
    auto map = res[one];
    return map.find(two) != map.end();
}

/**
 * @param g The time-sliced DBLP network
 * @param metapath The metapath to use for feature generation
 * @return positive and negative documents representing pairs of
 * potentially-collaborating authors
 */
template <class Graph>
std::vector<corpus::document> create_docs(Graph& g)
{
    std::vector<graph::metapath> metapaths
        = {graph::metapath{"author -- paper -- venue -- paper -- author"},
           graph::metapath{"author -- paper -> paper -> paper -- author"},
           graph::metapath{"author -- paper -> paper -- author"},
           graph::metapath{"author -- paper -- author -- paper -- author"}};

    using node_pair = std::pair<node_id, node_id>;
    std::unordered_map<node_pair, corpus::document> docs = three_hop_authors(g);
    for (auto& mpath : metapaths)
    {
        std::cout << "Adding metapath feature " << mpath.text() << std::endl;
        graph::algorithm::metapath_measures<Graph> measures{g, mpath};
        auto pc = measures.path_count();
        for (auto& p : docs)
        {
            auto source = p.first.first;
            auto dest = p.first.second;
            p.second.increment(mpath.text(), pc[source][dest]);
        }
    }

    std::vector<corpus::document> ret_docs;
    std::unordered_map<std::string, uint64_t> feature_map;
    std::ofstream out{"dblp.dat"};
    std::ofstream mapout{"dblp.mapping"};
    for (auto& p : docs)
    {
        auto src = p.first.first;
        auto dest = p.first.second;
        // only add pairs that are not currently co-authors
        if (!coauthors(g, src, dest))
        {
            // save mapping
            ret_docs.push_back(p.second);
            mapout << g.node(src).name << " <-> " << g.node(dest).name
                   << std::endl;

            // save classifier input
            out << p.second.label();
            for (auto& count : p.second.counts())
            {
                auto id = get_id(count.first, feature_map);
                out << " " << id << ":" << count.second;
            }
            out << std::endl;
        }
    }
    return ret_docs;
}

int main(int argc, char* argv[])
{
    logging::set_cerr_logging();
    std::string prefix = "/home/sean/projects/meta-data/dblp/"; // testing

    graph::directed_graph<graph::dblp_node> g_before;
    graph::dblp_loader::load(g_before, prefix, 0, 2005);
    std::vector<corpus::document> before_docs = create_docs(g_before);

    graph::directed_graph<graph::dblp_node> g_after;
    graph::dblp_loader::load(g_after, prefix, 2006, 2012);
    // set_labels(before_docs, g_after);

    // write_dataset(before_docs);
}
