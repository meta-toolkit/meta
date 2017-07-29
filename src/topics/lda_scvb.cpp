/**
 * @file lda_scvb.cpp
 * @author Chase Geigle
 */

#include "meta/topics/lda_scvb.h"
#include "meta/index/postings_data.h"
#include "meta/util/progress.h"
#include <random>

namespace meta
{
namespace topics
{

lda_scvb::lda_scvb(const learn::dataset& docs, std::size_t num_topics,
                   double alpha, double beta, uint64_t minibatch_size)
    : lda_model{docs, num_topics},
      docs_view_{docs_},
      alpha_{alpha},
      beta_{beta},
      minibatch_size_{
          std::min(minibatch_size, static_cast<uint64_t>(docs_.size()))}
{
    // nothing
}

void lda_scvb::run(uint64_t num_iters, double)
{
    initialize();
    for (uint64_t iter = 0; iter < num_iters; ++iter)
    {
        docs_view_.shuffle();
        perform_iteration(iter + 1);
    }
}

void lda_scvb::initialize()
{
    // TODO: Don't actually iterate through whole dataset here
    doc_topic_count_.resize(docs_.size());
    topic_term_count_.resize(num_topics_);
    for (auto& v : topic_term_count_)
        v.resize(docs_.total_features());
    topic_count_.resize(num_topics_);
    doc_sizes_.resize(docs_.size());

    std::mt19937 rng{std::random_device{}()};
    printing::progress progress{" > Initialization: ", docs_.size()};
    for (const auto& doc : docs_)
    {
        progress(doc.id);

        doc_sizes_[doc.id] = doc_size(doc);
        doc_topic_count_[doc.id].resize(num_topics_);

        for (const auto& freq : doc.weights)
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
                doc_topic_count_[doc.id][k] += gamma[k];
                topic_count_[k] += gamma[k];
            }
        }
    }
}

void lda_scvb::perform_iteration(uint64_t iter)
{
    printing::progress progress{"Minibatch " + std::to_string(iter) + ": ",
                                minibatch_size_};

    std::vector<std::unordered_map<term_id, double>> batch_topic_term_count_(
        num_topics_);
    std::vector<double> batch_topic_count_(num_topics_, 0.0);
    std::vector<double> gamma(num_topics_);

    uint64_t j = 0;
    for (const auto& doc : docs_view_)
    {
        if (j++ >= minibatch_size_)
            break;

        progress(j);

        // burn-in phase
        double t = 0;
        for (const auto& freq : doc.weights)
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] = (topic_term_count_[k][freq.first] + beta_)
                           / (topic_count_[k] + docs_.total_features() * beta_)
                           * (doc_topic_count_[doc.id][k] + alpha_);
                sum += gamma[k];
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] /= sum;
                auto lr = 1.0 / std::pow(10 + t, 0.9);
                auto weight = std::pow(1 - lr, freq.second);
                doc_topic_count_[doc.id][k]
                    = weight * doc_topic_count_[doc.id][k]
                      + (1 - weight) * doc_sizes_.at(doc.id) * gamma[k];
            }
            t += freq.second;
        }

        // normal phase
        for (const auto& freq : doc.weights)
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma[k] = (topic_term_count_[k][freq.first] + beta_)
                           / (topic_count_[k] + docs_.total_features() * beta_)
                           * (doc_topic_count_[doc.id][k] + alpha_);
                sum += gamma[k];
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                // renormalize gamma
                gamma[k] /= sum;

                // compute the learning schedule
                auto lr = 1.0 / std::pow(10 + t, 0.9);
                auto weight = std::pow(1 - lr, freq.second);

                doc_topic_count_[doc.id][k]
                    = weight * doc_topic_count_[doc.id][k]
                      + (1 - weight) * doc_sizes_.at(doc.id) * gamma[k];

                batch_topic_term_count_[k][freq.first]
                    += docs_.size() * gamma[k];

                batch_topic_count_[k] += docs_.size() * gamma[k];
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
        for (term_id i{0}; i < docs_.total_features(); ++i)
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
           / (topic_count_.at(topic) + docs_.total_features() * beta_);
}

double lda_scvb::compute_doc_topic_probability(learn::instance_id doc,
                                               topic_id topic) const
{
    return (doc_topic_count_.at(doc).at(topic) + alpha_)
           / (doc_sizes_.at(doc) + num_topics_ * alpha_);
}

stats::multinomial<topic_id> lda_scvb::topic_distribution(doc_id doc) const
{
    // TODO: Replace the count vectors with a multinomial rather than creating
    // it here
    stats::multinomial<topic_id> result;
    for (topic_id tid{0}; tid < num_topics_; ++tid)
    {
        result.increment(tid, doc_topic_count_.at(doc).at(tid) + alpha_);
    }

    return result;
}

stats::multinomial<term_id> lda_scvb::term_distribution(topic_id k) const
{
    // TODO: Replace the count vectors with a multinomial rather than creating
    // it here
    stats::multinomial<term_id> result;
    for (term_id w{0}; w < docs_.total_features(); ++w)
    {
        result.increment(w, topic_term_count_.at(k).at(w) + beta_);
    }

    return result;
}
}
}
