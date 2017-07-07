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

int print_topics(std::shared_ptr<index::cached_index<index::forward_index,
                                                     caching::no_evict_cache>>
                     idx,
                 topic_model tm)
{
    auto num_topics = tm.num_topics();
    for (term_id i{0}; i < num_topics; ++i)
    {
        std::cout << "Topic " << i << ":" << std::endl;
        std::cout << "-----------------" << std::endl;

        auto top_k = tm.top_k(i, 10);
        for (auto& i : top_k)
        {
            std::cout << idx->term_text(term_id{i.tid}) << " (" << i.tid
                      << "): " << i.probability << std::endl;
        }
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
    auto index
        = index::make_index<index::forward_index, caching::no_evict_cache>(
            *config);
    auto topic_model = topics::load_topic_model(*config);

    print_topics(index, topic_model);
}
