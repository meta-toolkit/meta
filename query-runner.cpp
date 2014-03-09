/**
 * @file query-runner.cpp
 */

#include <vector>
#include <string>
#include <iostream>

#include "util/time.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "caching/all.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    auto idx = index::make_index<
        index::inverted_index, caching::default_dblru_cache>(argv[1],
                                                             uint32_t{100000});

    index::okapi_bm25 ranker;

    auto config = cpptoml::parse_file(argv[1]);
    auto query_path = config.get_as<std::string>("querypath");
    if (!query_path)
    {
        std::cerr << "config file needs a \"querypath\" parameter" << std::endl;
        return 1;
    }

    std::ifstream queries{*query_path + *config.get_as<std::string>("dataset")
                          + "-queries.txt"};
    std::string content;
    auto elapsed_seconds = common::time([&]()
    {
        size_t i = 1;
        while (queries.good() && i <= 500)
        {
            std::getline(queries, content);
            corpus::document query{"[user input]", doc_id{0}};
            query.content(content);
            std::cout << "Ranking query " << i++ << ": " << query.path()
                      << std::endl;

            auto ranking = ranker.score(*idx, query);
            std::cout << "Showing top 10 of " << ranking.size() << " results."
                      << std::endl;

            for (size_t i = 0; i < ranking.size() && i < 10; ++i)
                std::cout << (i + 1) << ". " << idx->doc_name(ranking[i].first)
                          << " " << ranking[i].second << std::endl;

            std::cout << std::endl;
        }
    });

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "ms"
              << std::endl;

    return 0;
}
