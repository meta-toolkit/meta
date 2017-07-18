/**
 * @file lda_topics.cpp
 * @author Chase Geigle
 * @author Matt Kelly
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "meta/caching/no_evict_cache.h"
#include "meta/index/forward_index.h"
#include "meta/logging/logger.h"
#include "meta/topics/topic_model.h"

using namespace meta;
using namespace meta::topics;

int print_topics(const index::forward_index& idx, topic_model tm,
                 int topics_count)
{
    auto num_topics = tm.num_topics();

    // First, compute the denominators for each term's normalized score
    std::vector<double> denoms;
    denoms.reserve(idx.unique_terms());
    for (term_id t_id{0}; t_id < idx.unique_terms(); ++t_id)
    {
        double denom = 1.0;
        for (topic_id j{0}; j < num_topics; ++j)
            denom *= tm.term_probability(j, t_id);
        denom = std::pow(denom, 1.0 / num_topics);
        denoms.push_back(denom);
    }

    for (topic_id i{0}; i < num_topics; ++i)
    {
        std::cout << "Topic " << i << ":" << std::endl;
        std::cout << "-----------------" << std::endl;

        auto top_k = tm.top_k(i, topics_count);
        for (const auto& i : top_k)
        {
            std::cout << idx.term_text(i.tid) << " (" << i.tid << "): "
                      << i.probability * std::log(i.probability / denoms[i.tid])
                      << std::endl;
        }

        std::cout << std::endl;
    }

    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto topics_cfg = config->get_table("lda");
    auto topics_count = topics_cfg->get_as<int>("display-topics").value_or(10);
    auto index = index::make_index<index::forward_index>(*config);
    auto topic_model = topics::load_topic_model(*config);

    print_topics(*index, topic_model, topics_count);
}
