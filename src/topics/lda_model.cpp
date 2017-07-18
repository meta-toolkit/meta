/**
 * @file lda_model.cpp
 * @author Chase Geigle
 */

#include <fstream>

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

void lda_model::save_doc_topic_distributions(std::ostream& stream) const
{
    io::packed::write(stream, docs_.size());
    io::packed::write(stream, num_topics_);

    for (const auto& d : docs_)
    {
        io::packed::write(stream, topic_distrbution(doc_id{d.id}));
    }
}

void lda_model::save_topic_term_distributions(std::ostream& stream) const
{
    io::packed::write(stream, num_topics_);
    io::packed::write(stream, docs_.total_features());

    for (topic_id j{0}; j < num_topics_; ++j)
    {
        for (term_id t_id{0}; t_id < docs_.total_features(); ++t_id)
        {
            io::packed::write(stream, compute_term_topic_probability(t_id, j));
        }
    }
}

void lda_model::save(const std::string& prefix) const
{
    std::ofstream theta_file{prefix + ".theta.bin", std::ios::binary};
    std::ofstream phi_file{prefix + ".phi.bin", std::ios::binary};

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
