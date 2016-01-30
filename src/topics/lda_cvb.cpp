/**
 * @file lda_cvb.cpp
 * @author Chase Geigle
 */

#include <random>
#include "meta/index/postings_data.h"
#include "meta/logging/logger.h"
#include "meta/topics/lda_cvb.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

lda_cvb::lda_cvb(std::shared_ptr<index::forward_index> idx, uint64_t num_topics,
                 double alpha, double beta)
    : lda_model{std::move(idx), num_topics}
{
    gamma_.resize(idx_->num_docs());

    // each theta_ is a multinomial over topics with a symmetric
    // Dirichlet(\alpha) prior
    theta_.reserve(idx_->num_docs());
    for (doc_id doc{0}; doc < idx_->num_docs(); ++doc)
    {
        theta_.emplace_back(stats::dirichlet<topic_id>{alpha, num_topics_});
        gamma_[doc].resize(idx_->doc_size(doc));
    }

    // each phi_ is a multinomial over terms with a symmetric
    // Dirichlet(\beta) prior
    phi_.reserve(num_topics_);
    for (topic_id topic{0}; topic < num_topics_; ++topic)
        phi_.emplace_back(
            stats::dirichlet<term_id>{beta, idx_->unique_terms()});
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
        std::string spacing(static_cast<std::size_t>(
                                std::max<std::streamoff>(0, 80 - ss.tellp())),
                            ' ');
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

    for (doc_id d{0}; d < idx_->num_docs(); ++d)
    {
        progress(d);

        uint64_t i = 0; // i here is the inter-document term id, since we need
                        // to handle each word occurrence separately
        for (auto& freq : idx_->search_primary(d)->counts())
        {
            for (uint64_t count = 0; count < freq.second; ++count)
            {
                // create random gamma distributions
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    gamma_[d][i].increment(k, rng());
                }

                // contribute expected counts to phi_ and theta_
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    auto prob = gamma_[d][i].probability(k);
                    phi_[k].increment(freq.first, prob);
                    theta_[d].increment(k, prob);
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
                    // remove this word occurrence from the distributions
                    auto prob = gamma_[d][i].probability(k);
                    phi_[k].decrement(freq.first, prob);
                    theta_[d].decrement(k, prob);
                }

                gamma_[d][i].clear();
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    // "sample" the next topic: we are doing
                    // soft-assignment here so we actually just compute the
                    // probability of this topic
                    auto weight = phi_[k].probability(freq.first)
                                  * theta_[d].probability(k);
                    gamma_[d][i].increment(k, weight);
                }

                double delta = 0;
                for (topic_id k{0}; k < num_topics_; ++k)
                {
                    // recontribute expected counts, keep track of gamma
                    // changes for convergence
                    auto prob = gamma_[d][i].probability(k);
                    phi_[k].increment(freq.first, prob);
                    theta_[d].increment(k, prob);
                    delta += std::abs(prob - old_gamma.probability(k));
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
    return phi_[topic].probability(term);
}

double lda_cvb::compute_doc_topic_probability(doc_id doc, topic_id topic) const
{
    return theta_[doc].probability(topic);
}
}
}
