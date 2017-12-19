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
#include "meta/topics/bl_term_scorer.h"
#include "meta/topics/topic_model.h"

using namespace meta;
using namespace meta::topics;

int print_topics(const index::forward_index& idx, topic_model tm,
                 std::size_t topics_count)
{
    auto num_topics = tm.num_topics();

    for (topic_id i{0}; i < num_topics; ++i)
    {
        std::cout << "Topic " << i << ":" << std::endl;
        std::cout << "-----------------" << std::endl;

        topics::bl_term_scorer scorer{tm};
        auto top_k = tm.top_k(i, topics_count, scorer);
        for (const auto& i : top_k)
        {
            std::cout << idx.term_text(i.tid) << " (" << i.tid
                      << "): " << i.probability << std::endl;
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
    auto topics_count
        = topics_cfg->get_as<std::size_t>("display-topics").value_or(10);
    auto index = index::make_index<index::forward_index>(*config);
    auto topic_model = topics::load_topic_model(*config);

    print_topics(*index, topic_model, topics_count);
}
