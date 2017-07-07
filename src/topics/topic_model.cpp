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

topic_model::topic_model(const cpptoml::table& config, std::istream& theta,
                         std::istream& phi)
    : index_{index::make_index<index::forward_index, caching::no_evict_cache>(
          config)},
      num_topics_{io::packed::read<std::size_t>(phi)},
      num_words_{io::packed::read<std::size_t>(phi)},
      num_docs_{io::packed::read<std::size_t>(theta)},
      topic_term_probabilities_(num_topics_, util::aligned_vector<double>()),
      doc_topic_probabilities_(num_docs_, stats::multinomial<topic_id>())
{
    printing::progress term_progress{" > Loading topic term probabilities: ",
                                     num_topics_};
    for (std::size_t tid = 0; tid < num_topics_; ++tid)
    {
        if (!phi)
        {
            throw topic_model_exception{"topic term stream ended unexpectedly"};
        }

        term_progress(tid);
        auto& vec = topic_term_probabilities_[tid];
        vec.resize(num_words_);
        std::generate(vec.begin(), vec.end(),
                      [&]() { return io::packed::read<double>(phi); });
    }

    printing::progress doc_progress{" > Loading document topic probabilities: ",
                                    num_docs_};
    io::packed::read<std::size_t>(theta);
    for (std::size_t d = 0; d < num_docs_; ++d)
    {
        if (!theta)
        {
            throw topic_model_exception{
                "document topic stream ended unexpectedly"};
        }

        doc_progress(d);
        auto& dist = doc_topic_probabilities_[d];
        for (topic_id j{0}; j < num_topics_; ++j)
        {
            dist.increment(j, io::packed::read<double>(theta));
        }
    }
}

std::vector<term_prob> topic_model::top_k(term_id topic_id, std::size_t k) const
{
    auto pairs = util::make_fixed_heap<term_prob>(
        k, [](const term_prob& a, const term_prob& b) {
            return a.probability > b.probability;
        });

    auto current_topic = topic_term_probabilities_[topic_id];

    for (std::size_t i = 0; i < num_words_; ++i)
    {
        pairs.push(
            term_prob{i, index_->term_text(term_id{i}), current_topic[i]});
    }

    return pairs.extract_top();
}

stats::multinomial<topic_id> topic_model::topic_distribution(doc_id doc) const
{
    return doc_topic_probabilities_[doc];
}

term_prob topic_model::term_probability(topic_id topic_id,
                                        util::string_view term) const
{
    auto id = index_->get_term_id(term.to_string());
    auto prob = topic_term_probabilities_[topic_id][id];

    return {id, term.to_string(), prob};
}

topic_prob topic_model::topic_probability(doc_id doc, topic_id topic_id) const
{
    return {topic_id, doc_topic_probabilities_[doc].probability(topic_id)};
}

const std::size_t& topic_model::num_topics() const
{
    return num_topics_;
}

topic_model load_topic_model(const cpptoml::table& config)
{
    auto topics_cfg = config.get_table("lda");
    if (!topics_cfg)
    {
        throw topic_model_exception{
            "Missing [lda] configuration in configuration file"};
    }

    auto prefix = topics_cfg->get_as<std::string>("model-prefix");
    if (!prefix)
    {
        throw topic_model_exception{"Missing prefix key in configuration file"};
    }

    std::ifstream theta{*prefix + ".theta", std::ios::binary};
    std::ifstream phi{*prefix + ".phi", std::ios::binary};

    if (!theta)
    {
        throw topic_model_exception{
            "missing document topic probabilities file:" + *prefix + ".theta"};
    }

    if (!phi)
    {
        throw topic_model_exception{
            "missing topic term probabilities file:" + *prefix + ".phi"};
    }

    return topic_model{config, theta, phi};
}
}
}