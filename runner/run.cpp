//
// Created by Collin Gress on 10/30/16.
//

#include <iostream>
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/index/feedback/feedback_factory.h"
#include "meta/index/feedback/rocchio.h"

using namespace meta;

int main(int argc, char* argv[])
{
    auto config = cpptoml::parse_file(argv[1]);
    auto feedback_group = config->get_table("feedback");

    auto idx = index::make_index<index::dblru_inverted_index>(*config, 30000);
    auto fwd = index::make_index<index::forward_index>(*config);

    std::string test_query = "free time";
    corpus::document d;
    d.content(test_query);

    // note that when tokenized, "default-unigram-chain" filtering is done
    auto counts = idx->tokenize(d);

    // TODO: make this not need to be called explicitly
    d.vsm_vector().from_feature_map(counts, *idx);

    auto ranker_group = config->get_table("ranker");
    auto ranker = index::make_ranker(*ranker_group);


    /* TODO: attach this to the ranker->score function somehow
     * so we dont need to explicitly call apply_feedback (or even make feedback).
     * ideally, this functionality will work right "out of the box" assuming
     * a feedback implementation is defined in config.toml
     */
    std::vector<index::search_result> rankings = ranker->score(*idx, d, 50);

    // TODO: explicit feedback?
    // TODO: forward index support?

    auto feedback_impl = index::make_feedback(*feedback_group);
    corpus::document transformed = feedback_impl->apply_feedback(d, rankings, *fwd, *idx);

    // TODO: make rankers able to handle vector representation (i.e. work with the below call)
    rankings = ranker->score(*idx, transformed, 50);

    return 0;
}