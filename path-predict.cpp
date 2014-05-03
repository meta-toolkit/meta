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
        std::sort(sorted.begin(), sorted.end());

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
    //auto docs = partition(orig_docs);
    create_dataset(orig_docs);

    graph::algorithm::path_predict_eval pp_eval{"pp-config.toml"};
    //pp_eval.predictions();
    pp_eval.rankings();
}
