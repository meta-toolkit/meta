#include "meta/corpus/document.h"
#include "meta/index/ranker/all.h"
#include "meta/index/forward_index.h"

#include <iostream>

#include "meta/index/inverted_index.h"
#include "meta/logging/logger.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/time.h"

using namespace meta;


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    // Turn on logging to std::cerr.
    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    // Creates an inverted index with no cache. We don't need a cache here
    //  since we're never searching the index, only building it.
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::inverted_index>(*config);

    // Time how long it takes to create the index. By default, common::time's
    //  unit of measurement is milliseconds.
    auto time = common::time([&]()
    {
        // Create and make score of optimizer
        index::digamma_rec ranker;
        std::cout << ranker.get_optimized_mu(*idx) << std::endl;
    });

    std::cout << "Method DR took: " << time.count() / 1000.0
              << " seconds" << std::endl;

//    time = common::time([&]()
//    {
//        // Create and make score of optimizer
//        index::log_approx ranker;
//        std::cout << ranker.get_optimized_mu(*idx) << std::endl;
//    });

//    std::cout << "Method LA took: " << time.count() / 1000.0
//              << " seconds" << std::endl;

//    time = common::time([&]()
//    {
//        // Create and make score of optimizer
//        index::mackay_peto ranker;
//        std::cout << ranker.get_optimized_mu(*idx) << std::endl;
//    });

    std::cout << "Method MP took: " << time.count() / 1000.0
              << " seconds" << std::endl;

    return 0;
}
