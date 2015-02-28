/**
 * @file interactive_search.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/ranker_factory.h"
#include "parser/analyzers/tree_analyzer.h"
#include "sequence/analyzers/ngram_pos_analyzer.h"
#include "util/printing.h"
#include "util/time.h"

using namespace meta;

/**
 * @param path The path to the file to open
 * @return the text content of that file
 */
std::string get_content(const std::string& path)
{
    std::ifstream in{path};
    std::string str{(std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>()};
    std::replace(str.begin(), str.end(), '\n', ' ');
    return str;
}

/**
 * Demo app to allow a user to create queries and search an index.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    // Turn on logging to std::cerr.
    logging::set_cerr_logging();

    // Register additional analyzers.
    parser::register_analyzers();
    sequence::register_analyzers();

    // Create an inverted index using a splay cache. The arguments forwarded
    //  to make_index are the config file for the index and any parameters
    //  for the cache. In this case, we set the maximum number of nodes in
    //  the splay_cache to be 10000.
    auto idx = index::make_index<index::splay_inverted_index>(argv[1], 10000);

    // Create a ranking class based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto group = config.get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Find the path prefix to each document so we can print out the contents.
    std::string prefix = *config.get_as<std::string>("prefix")
                       + "/" + *config.get_as<std::string>("dataset") + "/";

    std::cout << "Enter a query, or blank to quit." << std::endl << std::endl;

    std::string text;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, text);

        if (text.empty())
            break;

        corpus::document query{"[user input]", doc_id{0}};
        query.content(text); // set the doc's content to be user input

        // Use the ranker to score the query over the index.
        std::vector<std::pair<doc_id, double>> ranking;
        auto time = common::time([&]()
        { ranking = ranker->score(*idx, query, 5); });

        std::cout << "Showing top 5 of results (" << time.count() << "ms)"
                  << std::endl;

        for (size_t i = 0; i < ranking.size() && i < 5; ++i)
        {
            std::string path{idx->doc_path(ranking[i].first)};
            std::cout << printing::make_bold(std::to_string(i + 1) + ". " + path
                                             + " ("
                                             + std::to_string(ranking[i].second)
                                             + ")") << std::endl;
            std::cout << get_content(prefix + path) << std::endl << std::endl;
        }

        std::cout << std::endl;
    }
}
