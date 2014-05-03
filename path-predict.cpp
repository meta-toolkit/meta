/**
 * @file path-predict.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include <random>
#include "graph/algorithm/path_predict_eval.h"
#include "graph/algorithm/path_predict.h"
#include "logging/logger.h"
#include "util/printing.h"

using namespace meta;

/**
 * @param orig_docs The original dataset
 * @return a vector of evenly-partitioned documents
 */
std::vector<corpus::document> partition(const std::vector
                                        <corpus::document>& orig_docs)
{
    std::vector<corpus::document> pos;
    std::vector<corpus::document> neg;
    for (auto& d : orig_docs)
    {
        if (d.label() == class_label{"0"})
            neg.push_back(d);
        else
            pos.push_back(d);
    }

    std::mt19937 gen(1);
    std::shuffle(neg.begin(), neg.end(), gen);
    neg.erase(neg.begin() + pos.size(), neg.end());

    pos.insert(pos.end(), neg.begin(), neg.end());
    return pos;
}

/**
 * @param docs The collection of documents to write to a file in libsvm format
 */
void create_dataset(const std::vector<corpus::document>& docs)
{
    std::unordered_map<std::string, uint64_t> feature_map;
    using pair_t = std::pair<std::string, double>;
    std::ofstream out{"pp/pp.dat"};
    std::ofstream out_map{"pp/pp.mapping"};
    for (auto& doc : docs)
    {
        size_t feature = 1;
        out << doc.label();
        std::vector<pair_t> sorted{doc.counts().begin(), doc.counts().end()};
        std::sort(sorted.begin(), sorted.end(),
                  [&](const pair_t& a, const pair_t& b)
        { return a.first < b.first; });

        for (auto& count : sorted)
            out << " " << feature++ << ":" << count.second;
        out << std::endl;
        out_map << doc.name() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    graph::algorithm::path_predict ppredict{argv[1]};
    auto orig_docs = ppredict.docs();
    auto docs = partition(orig_docs);
    create_dataset(docs);

    graph::algorithm::path_predict_eval pp_eval{"pp-config.toml"};
    pp_eval.predictions();
    pp_eval.rankings();
}
