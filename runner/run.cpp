//
// Created by Collin Gress on 10/30/16.
//

#include <iostream>
#include "meta/index/inverted_index.h"
#include "meta/index/ranker/ranker_factory.h"
#include "meta/index/feedback/feedback_factory.h"
#include "meta/index/eval/ir_eval.h"

using namespace meta;

int main(int argc, char* argv[])
{
    auto config = cpptoml::parse_file(argv[1]);
    auto path = (*config).get_as<std::string>("query-judgements");
    auto feedback_group = config->get_table("feedback");
    auto idx = index::make_index<index::dblru_inverted_index>(*config, 30000);
    auto fwd = index::make_index<index::forward_index>(*config);
    auto ranker_group = config->get_table("ranker");
    auto ranker = index::make_ranker(*ranker_group);
    auto feedback_impl = index::make_feedback(*feedback_group);
    std::ifstream queries{"../data/mas/mas-queries.txt"};

    auto eval = index::ir_eval(*config);

    int i = 0;

    queries.seekg(0, std::ios::beg);
    while(queries.good() && i < 70) {
        std::string query;
        std::getline(queries, query);

        corpus::document d{doc_id{i}};
        d.content(query);

        auto rankings = ranker->score(*idx, d, 50);
        auto transformed = feedback_impl->apply_feedback(d, rankings, *fwd, *idx);
        rankings = ranker->score_vsm(*idx, transformed, 50);

        i++;
    }
    std::cout << eval.map() << std::endl;

    return 0;
}