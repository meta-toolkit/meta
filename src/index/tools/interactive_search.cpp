/**
 * @file interactive_search.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "meta/corpus/document.h"
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/printing.h"
#include "meta/util/time.h"

using namespace meta;

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

    // Create an inverted index based on the config file.
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::inverted_index>(*config);

    // Create a ranking class based on the config file.
    auto group = config->get_table("ranker");
    if (!group)
        throw std::runtime_error{"\"ranker\" group needed in config file!"};
    auto ranker = index::make_ranker(*group);

    // Find the path prefix to each document so we can print out the contents.
    std::string prefix = *config->get_as<std::string>("prefix") + "/"
                         + *config->get_as<std::string>("dataset") + "/";

    std::cout << "Enter a query, or blank to quit." << std::endl << std::endl;

    std::string text;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, text);

        if (text.empty())
            break;

        corpus::document query{doc_id{0}};
        query.content(text); // set the doc's content to be user input

        // Use the ranker to score the query over the index.
        std::vector<index::search_result> ranking;
        auto time = common::time([&]()
                                 {
                                     ranking = ranker->score(*idx, query, 5);
                                 });

        std::cout << "Showing top 5 results (" << time.count() << "ms)"
                  << std::endl;

        uint64_t result_num = 1;
        for (auto& result : ranking)
        {
            std::string path{idx->doc_path(result.d_id)};
            auto output
                = printing::make_bold(std::to_string(result_num) + ". " + path)
                  + " (score = " + std::to_string(result.score) + ", docid = "
                  + std::to_string(result.d_id) + ")";
            std::cout << output << std::endl;
            auto mdata = idx->metadata(result.d_id);
            if (auto content = mdata.get<std::string>("content"))
            {
                auto len
                    = std::min(std::string::size_type{77}, content->size());
                std::cout << content->substr(0, len) << "..." << std::endl
                          << std::endl;
            }
            if (result_num++ == 5)
                break;
        }
    }
}
