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
#include "index/ranker/all.h"
#include "caching/dblru_cache.h"
#include "caching/shard_cache.h"
#include "caching/splay_cache.h"
#include "caching/unordered_map_cache.h"

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

    //auto idx = index::make_index<index::inverted_index>(argv[1]);
    auto idx = index::make_index<index::inverted_index,
                                 caching::splay_cache>(argv[1], uint32_t{200});
    //                           caching::lock_free_dblru_cache>(argv[1], uint64_t{2048});
    //                           caching::unordered_dblru_cache>(argv[1], uint64_t{2048});
    //                           caching::unordered_dblru_shard_cache>(argv[1], uint8_t{8}, uint64_t{2048});
    //                           caching::lock_free_dblru_shard_cache>(argv[1], uint8_t{8}, uint64_t{2048});
    //                           caching::splay_shard_cache>(argv[1], uint8_t{8}, uint32_t{10});
    index::okapi_bm25 ranker;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    std::cout << "Beginning ranking..." << std::endl;
    auto range = util::range<size_t>(0, std::min<size_t>(1000, idx.num_docs()-1));
    parallel::parallel_for(range.begin(), range.end(), [&](size_t i) {
        auto d_id = idx.docs()[i];
        index::document query{idx.doc_path(d_id)};
        auto ranking = ranker.score(idx, query);
    });

//  for(size_t i = 0; i < 100 && i < idx.num_docs(); ++i)
//  {
//      auto d_id = idx.docs()[i];
//      index::document query{idx.doc_path(d_id)};
//      cout << "Ranking query " << (i + 1) << ": " << query.path() << endl;

//      std::vector<std::pair<doc_id, double>> ranking = ranker.score(idx, query);
//      cout << "Showing top 10 of " << ranking.size() << " results." << endl;

//      for(size_t i = 0; i < ranking.size() && i < 10; ++i)
//          cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
//               << " " << ranking[i].second << endl;

//      cout << endl;
//  }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "Finished at " << std::ctime(&end_time)
              << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    return 0;
}
