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
    dblp_loader::load(g_after_, path, t1_start, t1_end);
    create_docs();
}

auto path_predict::three_hop_authors() -> std::unordered_map
    <node_pair, corpus::document>
{
    std::unordered_map<node_pair, corpus::document> docs;
    metapath_measures<graph_t> measures{
        g_before_,
        metapath{
            "author -- paper -- author -- paper -- author -- paper -- author"}};
    // g_before_, metapath{"author -- paper -- author -- paper -- author"}};
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

bool path_predict::coauthors(node_id one, node_id two, graph_t& g)
{
    metapath_measures<graph_t> meas{g, metapath{"author -- paper -- author"}};
    typename metapath_measures<graph_t>::measure_result res;
    meas.bfs_match(one, one, res, 0);
    auto map = res[one];
    return map.find(two) != map.end();
}

void path_predict::create_docs()
{
    std::vector<metapath> metapaths
        = {metapath{"author -- paper -> paper -- author"},
           metapath{"author -- paper <- paper -- author"},
           metapath{"author -- paper -- venue -- paper -- author"},
           metapath{"author -- paper -- term -- paper -- author"},
           metapath{"author -- paper -- author -- paper -- author"},
           metapath{"author -- paper -> paper -> paper -- author"},
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
        // auto pc = measures.path_count();
        auto pc = measures.normalized_path_count();
        // auto pc = measures.random_walk();
        // auto pc = measures.symmetric_random_walk();
        for (auto& p : hop_docs)
        {
            auto source = p.first.first;
            auto dest = p.first.second;
            p.second.increment(mpath.text(), pc[source][dest]);
        }
        std::cout << std::endl;
    }

    for (auto& p : hop_docs)
    {
        auto src = p.first.first;
        auto dest = p.first.second;
        // only add pairs that are not currently co-authors
        if (!coauthors(src, dest, g_before_))
        {
            // save mapping
            std::string dname = g_before_.node(src).name + "\t"
                                + g_before_.node(dest).name;
            p.second.name(dname);

            // save classifier input
            if (coauthors(src, dest, g_after_))
                p.second.label(class_label{"1"});
            else
                p.second.label(class_label{"0"});

            docs_.push_back(p.second);
        }
    }
}

std::vector<corpus::document> path_predict::docs() const
{
    return docs_;
}

}
}
}
