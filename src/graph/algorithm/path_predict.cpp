/**
 * @file path_predict.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "logging/logger.h"
#include "graph/algorithm/path_predict.h"

namespace meta
{
namespace graph
{
namespace algorithm
{

path_predict::path_predict(const std::string& config_file)
{
    auto config = cpptoml::parse_file(config_file);
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw path_predict_exception{"prefix missing from config"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw path_predict_exception{"dataset missing from config"};

    auto path = *prefix + "/" + *dataset + "/";

    auto group = config.get_group("path-predict");
    if (!group)
        throw path_predict_exception{
            "\"path-predict\" group missing from config"};

    auto t0_end = group->get_as<int64_t>("t0-end");
    if (!t0_end)
        throw path_predict_exception{"t0-end missing from path-predict config "
                                     "(this is the only time required)"};

    uint64_t t0_start = 0;
    if (auto time = group->get_as<int64_t>("t0-start"))
        t0_start = *time;

    uint64_t t1_start = *t0_end + 1;
    if (auto time = group->get_as<int64_t>("t1-start"))
        t1_start = *time;

    uint64_t t1_end = std::numeric_limits<uint64_t>::max();
    if (auto time = group->get_as<int64_t>("t1-end"))
        t1_end = *time;

    LOG(info) << "Running PathPredict\n data: " << path
              << "\n t0 start: " << t0_start << "\n t0 end:   " << *t0_end
              << "\n t1 start: " << t1_start << "\n t1 end:   " << t1_end
              << ENDLG;

    dblp_loader::load(g_before_, path, t0_start, *t0_end);
    create_docs();
    dblp_loader::load(g_after_, path, t1_start, t1_end);
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
    std::vector<metapath> metapaths
        = {metapath{"author -- paper -> paper -- author"},
           metapath{"author -- paper <- paper -- author"},
       //  metapath{"author -- paper -- venue -- paper -- author"},
       //  metapath{"author -- paper -- author -- paper -- author"},
       //  metapath{"author -- paper -> paper -> paper -- author"},
           metapath{"author -- paper <- paper <- paper -- author"},
           metapath{"author -- paper -> paper <- paper -- author"},
           metapath{"author -- paper <- paper -> paper -- author"}};

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
