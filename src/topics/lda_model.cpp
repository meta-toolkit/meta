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

lda_model::lda_model(const learn::dataset& docs, std::size_t num_topics)
    : docs_(docs), num_topics_{num_topics}
{
    /* nothing */
}

void lda_model::save_doc_topic_distributions(std::ostream& stream) const
{
    io::packed::write(stream, docs_.size());
    io::packed::write(stream, num_topics_);

    for (const auto& d : docs_)
    {
        auto dist = topic_distrbution(doc_id{d.id});
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
    denoms.reserve(docs_.total_features());
    for (term_id t_id{0}; t_id < docs_.total_features(); ++t_id)
    {
        double denom = 1.0;
        for (topic_id j{0}; j < num_topics_; ++j)
            denom *= compute_term_topic_probability(t_id, j);
        denom = std::pow(denom, 1.0 / num_topics_);
        denoms.push_back(denom);
    }

    io::packed::write(stream, num_topics_);
    io::packed::write(stream, docs_.total_features());

    // then, calculate and save each term's score
    for (topic_id j{0}; j < num_topics_; ++j)
    {
        for (term_id t_id{0}; t_id < docs_.total_features(); ++t_id)
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

std::size_t lda_model::doc_size(const learn::instance& inst)
{
    using pair_t = std::pair<learn::feature_id, double>;
    auto sum = std::accumulate(
        inst.weights.begin(), inst.weights.end(), 0.0,
        [](std::size_t amt, const pair_t& in) { return in.second + amt; });
    return static_cast<uint64_t>(sum);
}
}
}
