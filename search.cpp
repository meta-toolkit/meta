/**
 * @file search.cpp
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "caching/all.h"
#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/ranker/all.h"
#include "parallel/parallel_for.h"
#include "analyzers/analyzer.h"
#include "util/range.h"
#include "util/time.h"

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

    logging::set_cerr_logging();
    auto idx = index::make_index<index::inverted_index,
    //                           caching::splay_cache>(argv[1], uint32_t{10000});
                                 caching::default_dblru_cache>(argv[1], uint64_t{10000});
    //                           caching::lock_free_dblru_cache>(argv[1], uint64_t{2048});
    //                           caching::locking_dblru_cache>(argv[1], uint64_t{2048});
    //                           caching::locking_dblru_shard_cache>(argv[1], uint8_t{8}, uint64_t{2048});
    //                           caching::lock_free_dblru_shard_cache>(argv[1], uint8_t{8}, uint64_t{2048});
    //                           caching::splay_shard_cache>(argv[1], uint8_t{8}, uint32_t{10});
    index::pivoted_length ranker;
    //index::okapi_bm25 ranker;

    auto config = cpptoml::parse_file(argv[1]);
    std::string encoding = "utf-8";
    if (auto enc = config.get_as<std::string>("encoding"))
        encoding = *enc;

    auto elapsed = common::time([&](){
        // std::cout << "Beginning ranking..." << std::endl;
        // auto range = util::range<size_t>(0, std::min<size_t>(1000, idx.num_docs()-1));
        // parallel::parallel_for(range.begin(), range.end(), [&](size_t i) {
        //     auto d_id = idx.docs()[i];
        //     corpus::document query{idx.doc_path(d_id)};
        //     auto ranking = ranker.score(idx, query);
        // });

        for(size_t i = 0; i < 100 && i < idx.num_docs(); ++i)
        {
            auto d_id = idx.docs()[i];
            corpus::document query{idx.doc_path(d_id), doc_id{0}};
            query.set_encoding(encoding);
            cout << "Ranking query " << (i + 1) << ": " << query.path() << endl;

            std::vector<std::pair<doc_id, double>> ranking = ranker.score(idx, query);
            cout << "Showing top 10 of " << ranking.size() << " results." << endl;

            for(size_t i = 0; i < ranking.size() && i < 10; ++i)
                cout << (i+1) << ". " << idx.doc_name(ranking[i].first)
                     << " " << ranking[i].second << endl;

            cout << endl;
        }
    });

#if META_HAS_STD_PUT_TIME
    auto time =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "Finished at " << std::put_time(std::localtime(&time), "%c")
              << "\nElapsed time: " << elapsed.count() << "ms\n";
#else
    std::cout << "Elapsed time: " << elapsed.count() << "ms\n";
#endif

    return 0;
}
