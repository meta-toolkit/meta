/**
 * @file lda_cvb.cpp
 * @author Chase Geigle
 */

#include <random>
#include "index/postings_data.h"
#include "topics/lda_cvb.h"
#include "util/progress.h"

namespace meta
{
namespace topics
{

lda_cvb::lda_cvb(std::shared_ptr<index::forward_index> idx, uint64_t num_topics,
                 double alpha, double beta)
    : lda_model{std::move(idx), num_topics}, alpha_{alpha}, beta_{beta}
{
    /* nothing */
}

void lda_cvb::run(uint64_t num_iters, double convergence)
{
    initialize();
    for (uint64_t i = 0; i < num_iters; ++i)
    {
        std::stringstream ss;
        double max_change = perform_iteration(i);
        ss << "Iteration " << i + 1
           << " maximum change in gamma: " << max_change;
        std::string spacing(std::max<int>(0, 80 - ss.tellp()), ' ');
        ss << spacing;
        LOG(progress) << '\r' << ss.str() << '\n' << ENDLG;
        if (max_change <= convergence)
        {
            LOG(progress) << "Found convergence after " << i + 1
                          << " iterations!\n";
            break;
        }
    }
    LOG(info) << "Finished maximum iterations, or found convergence!" << ENDLG;
}

void lda_cvb::initialize()
{
    std::random_device rdev;
    std::mt19937 rng(rdev());
    printing::progress progress{"Initialization: ", idx_->num_docs()};

    gamma_.resize(idx_->num_docs());
    doc_topic_count_.resize(idx_->num_docs());
    topic_term_count_.resize(num_topics_);
    for (auto& v : topic_term_count_)
        v.resize(idx_->unique_terms());
    topic_count_.resize(num_topics_);

    for (doc_id d{0}; d < idx_->num_docs(); ++d)
    {
        progress(d);

        doc_topic_count_[d].resize(num_topics_);
        gamma_[d].resize(idx_->doc_size(d));

        uint64_t i = 0; // i here is the inter-document term id, since we need
                        // to handle each word occurrence separately
        for (auto& freq : idx_->search_primary(d)->counts())
        {
            for (uint64_t count = 0; count < freq.second; ++count)
            {
                double sum = 0;
                gamma_[d][i].resize(num_topics_);
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    auto random = rng();
                    gamma_[d][i][k] = random;
                    sum += random;
                }
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    gamma_[d][i][k] /= sum;
                    topic_term_count_[k][freq.first] += gamma_[d][i][k];
                    doc_topic_count_[d][k] += gamma_[d][i][k];
                    topic_count_[k] += gamma_[d][i][k];
                }
                i += 1;
            }
        }
    }
}

double lda_cvb::perform_iteration(uint64_t iter)
{
    printing::progress progress{"Iteration " + std::to_string(iter) + ": ",
                                idx_->num_docs()};
    progress.print_endline(false);
    double max_change = 0;
    for (doc_id d{0}; d < idx_->num_docs(); ++d)
    {
        progress(d);

        uint64_t i = 0; // term number within document---constructed
                        // so that each occurrence of the same term
                        // can still be assigned a different topic
        for (auto& freq : idx_->search_primary(d)->counts())
        {
            for (uint64_t count = 0; count < freq.second; ++count)
            {
                auto old_gamma = gamma_[d][i];
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    // remove this word occurrence from expectations
                    topic_term_count_[k][freq.first] -= gamma_[d][i][k];
                    doc_topic_count_[d][k] -= gamma_[d][i][k];
                    topic_count_[k] -= gamma_[d][i][k];
                }

                double sum = 0;
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    // "sample" the next topic: we are doing soft-assignment here
                    // so we actually just compute the probability of this topic
                    gamma_[d][i][k] = (topic_term_count_[k][freq.first] + beta_)
                                      / (topic_count_[k] + num_words_ * beta_)
                                      * (doc_topic_count_[d][k] + alpha_);
                    sum += gamma_[d][i][k];
                }

                double delta = 0;
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    // renormalize and recontribute to expected counts
                    gamma_[d][i][k] /= sum;
                    topic_term_count_[k][freq.first] += gamma_[d][i][k];
                    doc_topic_count_[d][k] += gamma_[d][i][k];
                    topic_count_[k] += gamma_[d][i][k];
                    delta += std::abs(gamma_[d][i][k] - old_gamma[k]);
                }
                max_change = std::max(max_change, delta);
                i += 1;
            }
        }
    }
    return max_change;
}

double lda_cvb::compute_term_topic_probability(term_id term,
                                               topic_id topic) const
{
    return (topic_term_count_.at(topic).at(term) + beta_)
           / (topic_count_.at(topic) + num_words_ * beta_);
}

double lda_cvb::compute_doc_topic_probability(doc_id doc, topic_id topic) const
{
    return (doc_topic_count_.at(doc).at(topic) + alpha_)
           / (idx_->doc_size(doc) + num_topics_ * alpha_);
}
}
}
