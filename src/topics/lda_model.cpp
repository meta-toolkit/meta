/**
 * @file lda_model.cpp
 * @author Chase Geigle
 */

#include "meta/topics/lda_model.h"
#include <iostream>

namespace meta
{
namespace topics
{

lda_model::lda_model(std::shared_ptr<index::forward_index> idx,
                     std::size_t num_topics)
    : idx_{std::move(idx)},
      num_topics_{num_topics},
      num_words_(idx_->unique_terms())
{
    /* nothing */
}

void lda_model::save_doc_topic_distributions(std::ostream& stream) const
{
    io::packed::write(stream, idx_->docs().size());
    io::packed::write(stream, num_topics_);

    for (const auto& d_id : idx_->docs())
    {
        auto dist = topic_distrbution(d_id);
        for (topic_id j{0}; j < num_topics_; ++j)
        {
            io::packed::write(stream, dist.counts(j));
        }
    }
}

void lda_model::save_topic_term_distributions(std::ostream& stream) const
{
    // first, compute the denominators for each term's normalized score
    std::vector<double> denoms;
    denoms.reserve(idx_->unique_terms());
    for (term_id t_id{0}; t_id < idx_->unique_terms(); ++t_id)
    {
        double denom = 1.0;
        for (topic_id j{0}; j < num_topics_; ++j)
            denom *= compute_term_topic_probability(t_id, j);
        denom = std::pow(denom, 1.0 / num_topics_);
        denoms.push_back(denom);
    }

    io::packed::write(stream, num_topics_);
    io::packed::write(stream, idx_->unique_terms());

    // then, calculate and save each term's score
    for (topic_id j{0}; j < num_topics_; ++j)
    {
        for (term_id t_id{0}; t_id < idx_->unique_terms(); ++t_id)
        {
            double prob = compute_term_topic_probability(t_id, j);
            double norm_prob = prob * std::log(prob / denoms[t_id]);
            io::packed::write(stream, norm_prob);
        }
    }
}

void lda_model::save(const std::string& prefix) const
{
    std::ofstream theta_file{prefix + ".theta", std::ios::binary};
    std::ofstream phi_file{prefix + ".phi", std::ios::binary};

    save_doc_topic_distributions(theta_file);
    save_topic_term_distributions(phi_file);
}

uint64_t lda_model::num_topics() const
{
    return num_topics_;
}
}
}
