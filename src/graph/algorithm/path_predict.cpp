/**
 * @file path_predict.cpp
 * @author Sean Massung
 */

#include "graph/algorithm/path_predict.h"

namespace meta
{
namespace graph
{
namespace algorithm
{

path_predict::path_predict(const std::string& config_file)
{
    std::string prefix = "/home/sean/projects/meta-data/dblp/";
    dblp_loader::load(g_before_, prefix, 0, 2005);
    create_docs();
    dblp_loader::load(g_after_, prefix, 2006, 2012);
}

auto path_predict::three_hop_authors() -> std::unordered_map
    <node_pair, corpus::document>
{
    std::unordered_map<node_pair, corpus::document> docs;
    metapath_measures<graph_t> measures{
        g_before_, metapath{"author -- paper -- author -- paper -- author"}};
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

uint64_t path_predict::get_id(const std::string& feature)
{
    auto it = feature_map_.find(feature);
    if (it == feature_map_.end())
    {
        auto next_id = feature_map_.size();
        feature_map_[feature] = next_id;
        return next_id;
    }
    return it->second;
}

bool path_predict::coauthors(node_id one, node_id two, bool before)
{
    // TODO check if before or not
    metapath_measures
        <graph_t> measures{g_before_, metapath{"author -- paper -- author"}};
    typename metapath_measures<graph_t>::measure_result res;
    measures.bfs_match(one, one, res, 0);
    auto map = res[one];
    return map.find(two) != map.end();
}

void path_predict::create_docs()
{
    std::vector<metapath> metapaths = {
        metapath{"author -- paper -> paper -- author"},
        metapath{"author -- paper <- paper -- author"},
        metapath{"author -- paper -- venue -- paper -- author"},
        metapath{"author -- paper -- author -- paper -- author"},
        metapath{"author -- paper -> paper -> paper -- author"},
        metapath{"author -- paper <- paper <- paper -- author"},
        metapath{"author -- paper -> paper <- paper -- author"},
        metapath{"author -- paper <- paper -> paper -- author"}
    };

    auto hop_docs = three_hop_authors();
    std::cout << std::endl;
    for (auto& mpath : metapaths)
    {
        std::cout << "Adding metapath feature: \"" << mpath.text() << "\""
                  << std::endl;
        metapath_measures<graph_t> measures{g_before_, mpath};
        auto pc = measures.path_count();
        for (auto& p : hop_docs)
        {
            auto source = p.first.first;
            auto dest = p.first.second;
            p.second.increment(mpath.text(), pc[source][dest]);
        }
        std::cout << std::endl;
    }

    std::ofstream out{"dblp.dat"};
    std::ofstream mapout{"dblp.mapping"};
    for (auto& p : hop_docs)
    {
        auto src = p.first.first;
        auto dest = p.first.second;
        // only add pairs that are not currently co-authors
        if (!coauthors(src, dest))
        {
            // save mapping
            docs_.push_back(p.second);
            mapout << g_before_.node(src).name << " <-> "
                   << g_before_.node(dest).name << std::endl;

            // save classifier input
            out << p.second.label();
            for (auto& count : p.second.counts())
            {
                auto id = get_id(count.first);
                out << " " << id << ":" << count.second;
            }
            out << std::endl;
        }
    }
}
}
}
}
