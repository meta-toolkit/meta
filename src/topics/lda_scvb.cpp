/**
 * @file lda_scvb.cpp
 * @author Chase Geigle
 */

#include <random>
#include "meta/index/postings_data.h"
#include "meta/topics/lda_scvb.h"
#include "meta/util/progress.h"

namespace meta
{
namespace topics
{

lda_scvb::lda_scvb(std::shared_ptr<index::forward_index> idx,
                   uint64_t num_topics, double alpha, double beta,
                   uint64_t minibatch_size)
    : lda_model{std::move(idx), num_topics},
      alpha_{alpha},
      beta_{beta},
      minibatch_size_{std::min(minibatch_size, idx_->num_docs())}
{
    // nothing
}

void lda_scvb::run(uint64_t num_iters, double)
{
    std::mt19937 gen{std::random_device{}()};
    initialize(gen);
    auto docs = idx_->docs();
    for (uint64_t iter = 0; iter < num_iters; ++iter)
    {
        std::shuffle(docs.begin(), docs.end(), gen);
        perform_iteration(iter + 1, docs);
    }
}

void lda_scvb::initialize(std::mt19937& rng)
{
    // TODO: Don't actually iterate through whole dataset here
    doc_topic_count_.resize(idx_->num_docs());
    topic_term_count_.resize(num_topics_);
    for (auto& v : topic_term_count_)
        v.resize(idx_->unique_terms());
    topic_count_.resize(num_topics_);

    printing::progress progress{" > Initialization: ", idx_->num_docs()};
    for (doc_id d{0}; d < idx_->num_docs(); ++d)
    {
        progress(d);

        doc_topic_count_[d].resize(num_topics_);

        for (auto& freq : idx_->search_primary(d)->counts())
        {
            double sum = 0;
            std::vector<double> gamma(num_topics_);
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                auto random = rng();
                gamma[k] = random;
                sum += random;
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] = gamma[k] * freq.second / sum;
                topic_term_count_[k][freq.first] += gamma[k];
                doc_topic_count_[d][k] += gamma[k];
                topic_count_[k] += gamma[k];
            }
        }
    }
}

void lda_scvb::perform_iteration(uint64_t iter, const std::vector<doc_id>& docs)
{
    printing::progress progress{"Minibatch " + std::to_string(iter) + ": ",
                                minibatch_size_};

    std::vector<std::unordered_map<term_id, double>> batch_topic_term_count_(
        num_topics_);
    std::vector<double> batch_topic_count_(num_topics_, 0.0);
    std::vector<double> gamma(num_topics_);

    for (uint64_t j = 0; j < minibatch_size_; ++j)
    {
        progress(j);
        auto d = docs[j];
        // burn-in phase
        double t = 0;
        for (const auto& freq : idx_->search_primary(d)->counts())
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] = (topic_term_count_[k][freq.first] + beta_)
                           / (topic_count_[k] + num_words_ * beta_)
                           * (doc_topic_count_[d][k] + alpha_);
                sum += gamma[k];
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] /= sum;
                auto lr = 1.0 / std::pow(10 + t, 0.9);
                auto weight = std::pow(1 - lr, freq.second);
                doc_topic_count_[d][k]
                    = weight * doc_topic_count_[d][k]
                      + (1 - weight) * idx_->doc_size(d) * gamma[k];
            }
            t += freq.second;
        }

        // normal phase
        for (const auto& freq : idx_->search_primary(d)->counts())
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] = (topic_term_count_[k][freq.first] + beta_)
                           / (topic_count_[k] + num_words_ * beta_)
                           * (doc_topic_count_[d][k] + alpha_);
                sum += gamma[k];
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                // renormalize gamma
                gamma[k] /= sum;

                // compute the learning schedule
                auto lr = 1.0 / std::pow(10 + t, 0.9);
                auto weight = std::pow(1 - lr, freq.second);

                doc_topic_count_[d][k]
                    = weight * doc_topic_count_[d][k]
                      + (1 - weight) * idx_->doc_size(d) * gamma[k];

                batch_topic_term_count_[k][freq.first]
                    += idx_->num_docs() * gamma[k];

                batch_topic_count_[k] += idx_->num_docs() * gamma[k];
            }
            t += freq.second;
        }
    }
    progress.end();

    // compute the learning schedule
    auto lr = 10.0 / std::pow(1000 + iter * minibatch_size_, 0.9);

    // TODO: better weight decay here? We can represent the vectors as the
    // product of a scalar and a vector to efficiently scale by (1 - lr)
    // when the batch count is 0, and we can cancel the factor out in the
    // addition when it is nonzero. Not sure if this will help, but I think
    // it may...
    for (topic_id k{0}; k < num_topics_; ++k)
    {
        for (term_id i{0}; i < num_words_; ++i)
        {
            topic_term_count_[k][i]
                = (1 - lr) * topic_term_count_[k][i]
                  + lr * (batch_topic_term_count_[k][i] / minibatch_size_);
        }
        topic_count_[k] = (1 - lr) * topic_count_[k]
                          + lr * (batch_topic_count_[k] / minibatch_size_);
    }
}

double lda_scvb::compute_term_topic_probability(term_id term,
                                                topic_id topic) const
{
    return (topic_term_count_.at(topic).at(term) + beta_)
           / (topic_count_.at(topic) + num_words_ * beta_);
}

double lda_scvb::compute_doc_topic_probability(doc_id doc, topic_id topic) const
{
    return (doc_topic_count_.at(doc).at(topic) + alpha_)
           / (idx_->doc_size(doc) + num_topics_ * alpha_);
}
}
}
