/**
 * @file interactive_search.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "caching/all.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "analyzers/analyzer.h"
#include "util/printing.h"
#include "util/time.h"

using namespace meta;

std::string get_snippets(const std::string & filename, const std::string & text)
{
    std::ifstream in{filename};
    std::string str{(std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>()};
    std::replace(str.begin(), str.end(), '\n', ' ');
    return str;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    // Turn on logging to std::cerr.
    logging::set_cerr_logging();

    // Create an inverted index using a splay cache. The arguments forwarded
    //  to make_index are the config file for the index and any parameters
    //  for the cache. In this case, we set the maximum number of nodes in
    //  the splay_cache to be 10000.
    auto idx = index::make_index<index::splay_inverted_index>(argv[1], 10000);

    // Create a ranking class based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto group = config.get_group("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    std::cout << "Enter a query, or blank query to quit." << std::endl
              << std::endl;

    std::string text;
    while(true)
    {
        std::cout << "> ";
        std::getline(std::cin, text);

        if(text.empty())
            break;

        corpus::document query{"[user input]", doc_id{0}};
        query.content(text);

        std::vector<std::pair<doc_id, double>> ranking;
        auto time = common::time([&](){
            ranking = ranker->score(*idx, query);
        });

        std::cout << "Showing top 10 of " << ranking.size()
                  << " results (" << time.count() << "ms)" << std::endl;

        for(size_t i = 0; i < ranking.size() && i < 5; ++i)
        {
            std::string path{idx->doc_path(ranking[i].first)};
            std::cout << printing::make_bold(
                        std::to_string(i+1) + ". " + path + " ("
                        + std::to_string(ranking[i].second) + ")"
                    ) << std::endl;
            std::cout << get_snippets(path, text) << std::endl << std::endl;

        }

        std::cout << std::endl;
    }
}
