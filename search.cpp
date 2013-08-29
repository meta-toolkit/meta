/**
 * @file search.cpp
 */

#include <chrono>
#include <vector>
#include <string>
#include <iostream>

#include "util/common.h"
#include "tokenizers/tokenizer.h"
#include "index/document.h"
#include "index/inverted_index.h"
#include "index/okapi_bm25.h"

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

    auto idx = index::make_index<index::inverted_index>(argv[1]);
    index::okapi_bm25 ranker;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for(size_t i = 0; i < 100 && i < idx.num_docs(); ++i)
    {
        auto d_id = idx.docs()[i];
        index::document query{idx.doc_path(d_id)};
        std::vector<std::pair<doc_id, double>> ranking = ranker.score(idx, query);
        cout << "Query " << i << ": " << query.path() << endl;

        for(size_t i = 0; i < ranking.size() && i < 10; ++i)
            cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
                 << " " << ranking[i].second << endl;

        cout << endl;
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
     
    std::cout << "Finished at " << std::ctime(&end_time)
              << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
