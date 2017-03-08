/**
 * @file lda_model.cpp
 * @author Chase Geigle
 */

#include "meta/topics/lda_model.h"

namespace meta
{
namespace topics
{

lda_model::lda_model(const learn::dataset& docs, std::size_t num_topics)
    : docs_(docs), num_topics_{num_topics}
{
    /* nothing */
}

void lda_model::save_doc_topic_distributions(const std::string& filename) const
{
    std::ofstream file{filename};
    for (const auto& doc : docs_)
    {
        file << doc.id << "\t";
        double sum = 0;
        for (topic_id j{0}; j < num_topics_; ++j)
        {
            double prob = compute_doc_topic_probability(doc.id, j);
            if (prob > 0)
                file << j << ":" << prob << "\t";
            sum += prob;
        }
        if (std::abs(sum - 1) > 1e-6)
            throw std::runtime_error{"invalid probability distribution"};
        file << "\n";
    }
}

void lda_model::save_topic_term_distributions(const std::string& filename) const
{
    std::ofstream file{filename};

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

    // then, calculate and save each term's score
    for (topic_id j{0}; j < num_topics_; ++j)
    {
        file << j << "\t";
        for (term_id t_id{0}; t_id < docs_.total_features(); ++t_id)
        {
            double prob = compute_term_topic_probability(t_id, j);
            double norm_prob = prob * std::log(prob / denoms[t_id]);
            if (norm_prob > 0)
                file << t_id << ":" << norm_prob << "\t";
        }
        file << "\n";
    }
}

void lda_model::save(const std::string& prefix) const
{
    save_doc_topic_distributions(prefix + ".theta");
    save_topic_term_distributions(prefix + ".phi");
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
