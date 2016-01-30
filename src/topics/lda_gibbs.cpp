/**
 * @file lda_gibbs.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cmath>

#include "meta/index/postings_data.h"
#include "meta/logging/logger.h"
#include "meta/topics/lda_gibbs.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

lda_gibbs::lda_gibbs(std::shared_ptr<index::forward_index> idx,
                     uint64_t num_topics, double alpha, double beta)
    : lda_model{std::move(idx), num_topics}
{
    doc_word_topic_.resize(idx_->num_docs());

    // each theta_ is a multinomial over topics with a symmetric
    // Dirichlet(\alpha) prior
    theta_.reserve(idx_->num_docs());
    for (doc_id doc{0}; doc < idx_->num_docs(); ++doc)
    {
        theta_.emplace_back(stats::dirichlet<topic_id>{alpha, num_topics_});
        doc_word_topic_[doc].resize(idx_->doc_size(doc));
    }

    // each phi_ is a multinomial over terms with a symmetric
    // Dirichlet(\beta) prior
    phi_.reserve(num_topics_);
    for (topic_id topic{0}; topic < num_topics_; ++topic)
        phi_.emplace_back(
            stats::dirichlet<term_id>{beta, idx_->unique_terms()});

    std::random_device dev;
    rng_.seed(dev());
}

void lda_gibbs::run(uint64_t num_iters, double convergence /* = 1e-6 */)
{
    initialize();
    double likelihood = corpus_log_likelihood();
    std::stringstream ss;
    ss << "Initialization log likelihood (log P(W|Z)): " << likelihood;
    std::string spacing(
        static_cast<std::size_t>(std::max<std::streamoff>(0, 80 - ss.tellp())),
        ' ');
    ss << spacing;
    LOG(progress) << '\r' << ss.str() << '\n' << ENDLG;

    for (uint64_t i = 0; i < num_iters; ++i)
    {
        perform_iteration(i + 1);
        double likelihood_update = corpus_log_likelihood();
        double ratio = std::fabs((likelihood - likelihood_update) / likelihood);
        likelihood = likelihood_update;
        std::stringstream ss;
        ss << "Iteration " << i + 1
           << " log likelihood (log P(W|Z)): " << likelihood;
        std::string spacing(static_cast<std::size_t>(
                                std::max<std::streamoff>(0, 80 - ss.tellp())),
                            ' ');
        ss << spacing;
        LOG(progress) << '\r' << ss.str() << '\n' << ENDLG;
        if (ratio <= convergence)
        {
            LOG(progress) << "Found convergence after " << i + 1
                          << " iterations!\n" << ENDLG;
            break;
        }
    }
    LOG(info) << "Finished maximum iterations, or found convergence!" << ENDLG;
}

topic_id lda_gibbs::sample_topic(term_id term, doc_id doc)
{
    stats::multinomial<topic_id> full_conditional;
    for (topic_id topic{0}; topic < num_topics_; ++topic)
    {
        auto weight = compute_sampling_weight(term, doc, topic);
        full_conditional.increment(topic, weight);
    }
    return full_conditional(rng_);
}

double lda_gibbs::compute_sampling_weight(term_id term, doc_id doc,
                                          topic_id topic) const
{
    return compute_term_topic_probability(term, topic)
           * compute_doc_topic_probability(doc, topic);
}

double lda_gibbs::compute_term_topic_probability(term_id term,
                                                 topic_id topic) const
{
    return phi_[topic].probability(term);
}

double lda_gibbs::compute_doc_topic_probability(doc_id doc,
                                                topic_id topic) const
{
    return theta_[doc].probability(topic);
}

void lda_gibbs::initialize()
{
    perform_iteration(0, true);
}

void lda_gibbs::perform_iteration(uint64_t iter, bool init /* = false */)
{
    std::string str;
    if (init)
        str = "Initialization: ";
    else
        str = "Iteration " + std::to_string(iter) + ": ";
    printing::progress progress{str, idx_->num_docs()};
    progress.print_endline(false);
    for (const auto& i : idx_->docs())
    {
        progress(i);
        uint64_t n = 0; // term number within document---constructed
                        // so that each occurrence of the same term
                        // can still be assigned a different topic
        for (const auto& freq : idx_->search_primary(i)->counts())
        {
            for (uint64_t j = 0; j < freq.second; ++j)
            {
                auto old_topic = doc_word_topic_[i][n];
                // don't include current topic assignment in
                // probability calculation
                if (!init)
                    decrease_counts(old_topic, freq.first, i);

                // sample a new topic assignment
                auto topic = sample_topic(freq.first, i);
                doc_word_topic_[i][n] = topic;

                // increase counts
                increase_counts(topic, freq.first, i);
                n += 1;
            }
        }
    }
}

void lda_gibbs::decrease_counts(topic_id topic, term_id term, doc_id doc)
{
    phi_[topic].decrement(term, 1);
    theta_[doc].decrement(topic, 1);
}

void lda_gibbs::increase_counts(topic_id topic, term_id term, doc_id doc)
{
    phi_[topic].increment(term, 1);
    theta_[doc].increment(topic, 1);
}

double lda_gibbs::corpus_log_likelihood() const
{
    auto tid0 = topic_id{0};
    // V * \beta if symmetric, \sum_{r=1}^V \beta_r otherwise
    auto total_pcs = phi_[tid0].prior().pseudo_counts();

    double likelihood = num_topics_ * std::lgamma(total_pcs);

    for (topic_id j{0}; j < num_topics_; ++j)
    {
        for (term_id t{0}; t < num_words_; ++t)
        {
            likelihood += std::lgamma(phi_[j].counts(t))
                          - std::lgamma(phi_[j].prior().pseudo_counts(t));
        }
        likelihood -= std::lgamma(phi_[j].counts());
    }
    return likelihood;
}
}
}
