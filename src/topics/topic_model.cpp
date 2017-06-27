/**
* @file topic_model.h
* @author Matt Kelly
*
* All files in META are dual-licensed under the MIT and NCSA licenses. For more
* details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
* project.
*/

#include <iostream>

#include "meta/topics/topic_model.h"
#include "meta/io/packed.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

topic_model::topic_model(std::istream& theta, std::istream& phi)
	: num_topics_{ io::packed::read<std::size_t>(phi) },
	  num_words_{ io::packed::read<std::size_t>(phi) },
	  topic_term_probabilities_(num_topics_, util::aligned_vector<double>())
{
	std::cout << num_topics_ << std::endl;
	std::cout << num_words_ << std::endl;

	printing::progress progress{" > Loading topic term probabilities: ", num_topics_};
	for (std::size_t tid = 0; tid < num_topics_; ++tid)
	{

		progress(tid);
		auto& vec = topic_term_probabilities_[tid];
		vec.resize(num_words_);
		
		std::generate(vec.begin(), vec.end(), [&]() { return io::packed::read<double>(phi); });
	}
}

std::vector<term> topic_model::top_k(std::size_t topic_id, std::size_t k) const
{
	auto pairs = util::make_fixed_heap<term>(
		k, [](const term& a, const term& b) {
		return a.probability > b.probability;
	});

	auto current_topic = topic_term_probabilities_[topic_id];

	for (int i; i < num_words_; ++i)
	{
		pairs.push(term{ i, current_topic[i] });
	}

	return pairs.extract_top();
}

std::vector<topic> topic_model::topic_distribution(std::size_t doc) const
{

}

term topic_model::term_probability(std::size_t topic_id, util::string_view term) const
{

}

topic topic_model::topic_probability(std::size_t doc, std::size_t topic_id) const
{

}

const std::size_t topic_model::num_topics() const
{
	return num_topics_;
}

topic_model load_topic_model(const cpptoml::table& config)
{
	auto prefix = config.get_as<std::string>("model-prefix");
	if (!prefix)
		throw topic_model_exception{
			"missing prefix key in configuration file" };

	std::cout << *prefix << ".theta" << std::endl;
	std::ifstream theta{ *prefix + ".theta", std::ios::binary };
	std::ifstream phi{ *prefix + ".phi", std::ios::binary };

	return topic_model(theta, phi);
}

}
}