/**
 * @file lda_topics.cpp
 * @author Chase Geigle
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "meta/caching/no_evict_cache.h"
#include "meta/index/forward_index.h"
#include "meta/logging/logger.h"
#include "meta/topics/topic_model.h"
#include "meta/util/fixed_heap.h"

using namespace meta;
using namespace meta::topics;

int print_topics(const cpptoml::table& config, topic_model tm)
{
    auto idx = index::make_index<index::forward_index, caching::no_evict_cache>(config);
	 
	 auto num_topics = tm.num_topics();
	 for (int i = 0; i < num_topics; ++i)
	 {
		 std::cout << "Topic " << i << ":" << std::endl;
		 std::cout << "-----------------" << std::endl;

		 auto top_k = tm.top_k(i, 10);
		 std::cout << "-----------------" << std::endl;
		 for (auto& i : top_k)
		 {
			 std::cout << idx->term_text(term_id{ i.tid }) << " (" << i.tid << " ): " << i.probability << std::endl;
		 }
	 }

	 return 0;
}

int main(int argc, char** argv)
{
    //std::vector<std::string> args(argv, argv + argc);
    //if (args.size() < 4)
    //    return print_usage(args[0]);
    //return print_topics(args[1], args[2], std::stoul(argv[3]));

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
		return 1;
	}

	logging::set_cerr_logging();

	auto config = cpptoml::parse_file(argv[1]);
	auto topics_cfg = config->get_table("lda");
	if (!topics_cfg)
	{
		std::cerr << "Missing [lda] configuration in " << argv[1]
			<< std::endl;
		return 1;
	}

	auto topic_model = topics::load_topic_model(*topics_cfg);

	print_topics(*config, topic_model);
}
