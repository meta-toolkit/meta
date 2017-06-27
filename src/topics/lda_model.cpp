/**
 * @file lda_model.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include "meta/topics/lda_model.h"

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

void lda_model::save_doc_topic_distributions(const std::string& filename) const
{
    std::ofstream file{filename, std::ios::binary};
    for (const auto& d_id : idx_->docs())
    {
        io::packed::write(file, d_id);

        double sum = 0;
        for (topic_id j{0}; j < num_topics_; ++j)
        {
            double prob = compute_doc_topic_probability(d_id, j);
				if (prob > 0)
					io::packed::write(file, double(prob));
            sum += prob;
        }
        if (std::abs(sum - 1) > 1e-6)
            throw lda_model_excpetion{"invalid probability distribution"};
    }
}

void lda_model::save_topic_term_distributions(const std::string& filename) const
{
    std::ofstream file{filename, std::ios::binary};

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

	 io::packed::write(file, num_topics_);
	 io::packed::write(file, idx_->unique_terms());

    // then, calculate and save each term's score
    for (topic_id j{0}; j < num_topics_; ++j)
    {
		  std::cout << idx_->unique_terms() << std::endl;
		  //io::packed::write(file, j); 
        for (term_id t_id{0}; t_id < idx_->unique_terms(); ++t_id)
        {
            double prob = compute_term_topic_probability(t_id, j);
            double norm_prob = prob * std::log(prob / denoms[t_id]);
				std::cout << "Topic " << j << ": Id: " << t_id << " Probability:" << norm_prob << std::endl;
				//file << t_id << ":" << norm_prob << "\t";
				//io::packed::write(file, t_id);
				io::packed::write(file, norm_prob);
        }
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
}
}
