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
    : num_topics_{io::packed::read<std::size_t>(phi)},
      num_words_{io::packed::read<std::size_t>(phi)},
      num_docs_{io::packed::read<std::size_t>(theta)},
      topic_term_probabilities_(num_topics_),
      doc_topic_probabilities_(num_docs_)
{
    {
        printing::progress term_progress{
            " > Loading topic term probabilities: ", num_topics_};
        for (topic_id tid{0}; tid < num_topics_; ++tid)
        {
            if (!phi)
            {
                throw topic_model_exception{
                    "topic term stream ended unexpectedly"};
            }

            term_progress(tid);
            auto& dist = topic_term_probabilities_[tid];
            io::packed::read(phi, dist);
        }
    }

    {
        printing::progress doc_progress{
            " > Loading document topic probabilities: ", num_docs_};
        for (std::size_t d = 0; d < num_docs_; ++d)
        {
            if (!theta)
            {
                throw topic_model_exception{
                    "document topic stream ended unexpectedly"};
            }

            doc_progress(d);
            auto& dist = doc_topic_probabilities_[d];
            io::packed::read(theta, dist);
        }
    }
}

std::vector<term_prob> topic_model::top_k(topic_id tid, std::size_t k) const
{
    return top_k(tid, k, [this](topic_id t, term_id v) {
        return term_probability(t, v);
    });
}

const stats::multinomial<topic_id>&
topic_model::topic_distribution(doc_id doc) const
{
    return doc_topic_probabilities_[doc];
}

const stats::multinomial<term_id>&
topic_model::term_distribution(topic_id k) const
{
    return topic_term_probabilities_[k];
}

double topic_model::term_probability(topic_id top_id, term_id tid) const
{
    return topic_term_probabilities_[top_id].probability(tid);
}

double topic_model::topic_probability(doc_id doc, topic_id topic_id) const
{
    return doc_topic_probabilities_[doc].probability(topic_id);
}

std::size_t topic_model::num_topics() const
{
    return num_topics_;
}

std::size_t topic_model::num_words() const
{
    return num_words_;
}

std::size_t topic_model::num_docs() const
{
    return num_docs_;
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

    std::ifstream theta{*prefix + ".theta.bin", std::ios::binary};
    std::ifstream phi{*prefix + ".phi.bin", std::ios::binary};

    if (!theta)
    {
        throw topic_model_exception{"missing document topic probabilities file:"
                                    + *prefix + ".theta.bin"};
    }

    if (!phi)
    {
        throw topic_model_exception{
            "missing topic term probabilities file:" + *prefix + ".phi.bin"};
    }

    return topic_model{theta, phi};
}
}
}
