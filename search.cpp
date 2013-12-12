/**
 * @file search.cpp
 */

#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

#include "util/common.h"
#include "tokenizers/tokenizer.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "caching/all.h"

#include "util/range.h"
#include "parallel/parallel_for.h"

using namespace meta;
using std::cerr;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    auto idx = index::make_index<index::inverted_index,
                                 caching::splay_cache>(argv[1], uint32_t{10000});

    index::okapi_bm25 ranker;

    auto config = cpptoml::parse_file(argv[1]);
    std::string queryfile = "/home/sean/projects/cs446-project/index-comparison/queries/";
    std::ifstream queries{queryfile + *config.get_as<std::string>("dataset") + "-queries.txt"};
    std::string content;
    auto elapsed_seconds = common::time([&]()
    {
        size_t i = 1;
        while(queries.good() && i <= 500)
        {
            std::getline(queries, content);
            corpus::document query{"[user input]", doc_id{0}};
            query.set_content(content);
            cout << "Ranking query " << i++ << ": " << query.path() << endl;

            std::vector<std::pair<doc_id, double>> ranking = ranker.score(idx, query);
            cout << "Showing top 10 of " << ranking.size() << " results." << endl;

            for(size_t i = 0; i < ranking.size() && i < 10; ++i)
                cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
                     << " " << ranking[i].second << endl;

            cout << endl;
        }
    });

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "ms\n";

    return 0;
}
