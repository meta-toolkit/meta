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
      doc_topic_probabilities_(num_docs_, util::aligned_vector<double>())
{
    printing::progress term_progress{" > Loading topic term probabilities: ",
                                     num_topics_};
    for (std::size_t tid = 0; tid < num_topics_; ++tid)
    {
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
        doc_progress(d);
        auto& vec = doc_topic_probabilities_[d];
        vec.resize(num_topics_);
        std::generate(vec.begin(), vec.end(),
                      [&]() { return io::packed::read<double>(theta); });
    }
}

std::vector<term> topic_model::top_k(std::size_t topic_id, std::size_t k) const
{
    auto pairs
        = util::make_fixed_heap<term>(k, [](const term& a, const term& b) {
              return a.probability > b.probability;
          });

    auto current_topic = topic_term_probabilities_[topic_id];

    for (std::size_t i = 0; i < num_words_; ++i)
    {
        pairs.push(term{i, index_->term_text(term_id{i}), current_topic[i]});
    }

    return pairs.extract_top();
}

std::vector<topic> topic_model::topic_distribution(std::size_t doc) const
{
    auto doc_topics = doc_topic_probabilities_[doc];
    std::vector<topic> topic_probs = std::vector<topic>();

    for (std::size_t i = 0; i < doc_topics.size(); ++i)
    {
        topic_probs.push_back(topic{i, doc_topics[i]});
    }

    return topic_probs;
}

term topic_model::term_probability(std::size_t topic_id,
                                   util::string_view term) const
{
    auto id = index_->get_term_id(term.to_string());
    auto prob = topic_term_probabilities_[topic_id][id];

    return {id, term.to_string(), prob};
}

topic topic_model::topic_probability(std::size_t doc,
                                     std::size_t topic_id) const
{
    return {topic_id, doc_topic_probabilities_[doc][topic_id]};
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

    return topic_model{config, theta, phi};
}
}
}