//
// Created by Collin Gress on 10/30/16.
//

#include <iostream>
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/index/feedback/rocchio.h"

using namespace meta;

int main(int argc, char* argv[])
{
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::dblru_inverted_index>(*config, 30000);

    std::string test_query = "free time";
    corpus::document d;
    d.content(test_query);

    // note that when tokenized, "default-unigram-chain" filtering is done
    auto counts = idx->tokenize(d);
    d.vector().from_feature_map(counts, *idx);

    auto ranker_group = config->get_table("ranker");
    auto ranker = index::make_ranker(*ranker_group);

    auto rankings = ranker->score(*idx, d, 50);

    return 0;
}