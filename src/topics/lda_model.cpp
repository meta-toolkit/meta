/**
 * @file lda_model.cpp
 */

#include "topics/lda_model.h"

namespace meta
{
namespace topics
{

lda_model::lda_model(index::forward_index& idx, uint64_t num_topics)
    : idx_(idx), // gcc no non-const ref init from brace init list
      num_topics_{num_topics},
      num_words_{idx.unique_terms()}
{
    /* nothing */
}

void lda_model::save_doc_topic_distributions(const std::string& filename) const
{
    std::ofstream file{filename};
    for (const auto& d_id : idx_.docs())
    {
        file << d_id << "\t";
        for (topic_id j{0}; j < num_topics_; ++j)
        {
            double prob = compute_doc_topic_probability(d_id, j);
            if (prob > 0)
                file << j << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void lda_model::save_topic_term_distributions(const std::string& filename) const
{
    std::ofstream file{filename};
    for (topic_id j{0}; j < num_topics_; ++j)
    {
        file << j << "\t";
        for (term_id t_id{0}; t_id < idx_.unique_terms(); ++t_id)
        {
            double prob = compute_term_topic_probability(t_id, j);
            if (prob > 0)
                file << t_id << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void lda_model::save(const std::string& prefix) const
{
    save_doc_topic_distributions(prefix + ".theta");
    save_topic_term_distributions(prefix + ".phi");
}
}
}
