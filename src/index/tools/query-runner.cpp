/**
 * @file query-runner.cpp
 * @author Sean Massung
 */

#include <vector>
#include <string>
#include <iostream>

#include "util/time.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/ranker_factory.h"
#include "parser/analyzers/tree_analyzer.h"
#include "sequence/analyzers/ngram_pos_analyzer.h"

using namespace meta;

/**
 * Demo app to read a file with one query per line and run each query on an
 * inverted index.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    // Log to standard error
    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    // Create an inverted index using a DBLRU cache. The arguments forwarded to
    //  make_index are the config file for the index and any parameters for the
    //  cache. In this case, we set the maximum hash table size for the
    //  dblru_cache to be 10000.
    auto idx = index::make_index<index::dblru_inverted_index>(argv[1], 10000);

    // Create a ranking class based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto group = config.get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Get the path to the file containing queries
    auto query_path = config.get_as<std::string>("querypath");
    if (!query_path)
        throw std::runtime_error{"config file needs a \"querypath\" parameter"};

    std::ifstream queries{*query_path + *config.get_as<std::string>("dataset")
                          + "-queries.txt"};
    std::string content;
    auto elapsed_seconds = common::time([&]()
    {
        size_t i = 1;
        while (queries.good() && i <= 500) // only look at first 500 queries
        {
            std::getline(queries, content);
            corpus::document query{"[user input]", doc_id{0}};
            query.content(content);
            std::cout << "Ranking query " << i++ << ": " << query.path()
                      << std::endl;

            // Use the ranker to score the query over the index. By default, the
            //  ranker returns 10 documents, so we will display the "top 10 of
            //  10" docs.
            auto ranking = ranker->score(*idx, query);
            std::cout << "Showing top 10 of " << ranking.size() << " results."
                      << std::endl;

            for (size_t i = 0; i < ranking.size() && i < 10; ++i)
            {
                std::cout << (i + 1) << ". " << idx->doc_name(ranking[i].first)
                          << " " << ranking[i].second << std::endl;
            }
            std::cout << std::endl;
        }
    });

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "ms"
              << std::endl;

    return 0;
}
