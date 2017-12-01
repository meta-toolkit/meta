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


void display_result(float alpha, std::map<term_id, double> alpha_m, float time){
    for (auto kv: alpha_m){
        std::cout << kv.second << " ";
    }
    std::cout << std::endl << alpha << std::endl << time << std::endl;
}

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

    double eps = 1e-6;
    int iters = 10000;

    float alpha;
    std::map<term_id, double> alpha_m;

    index::dirichlet_digamma_rec ranker1;
    index::dirichlet_log_approx ranker2;
    index::dirichlet_mackay_peto ranker3;

    auto time1 = common::time([&]()
    {
        alpha_m = ranker1.get_optimized_mu(*idx, eps, iters);
        alpha = ranker1.parameter();
    });

    display_result(alpha, alpha_m, time1.count() / 1.0);

    auto time2 = common::time([&]()
    {
        alpha_m = ranker2.get_optimized_mu(*idx, eps, iters);
        alpha = ranker2.parameter();
    });

    display_result(alpha, alpha_m, time2.count() / 1.0);


    auto time3 = common::time([&]()
    {
        alpha_m = ranker3.get_optimized_mu(*idx, eps, iters);
        alpha = ranker3.parameter();
    });

    display_result(alpha, alpha_m, time3.count() / 1.0);

    return 0;
}
