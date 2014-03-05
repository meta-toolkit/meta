/**
 * @file lda_cvb.cpp
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
    for (doc_id i{0}; i < idx_->num_docs(); ++i)
    {
        progress(i);
        for (auto& freq : idx_->search_primary(i)->counts())
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                double random = rng();
                gamma_[i][freq.first][k] = random;
                sum += random;
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma_[i][freq.first][k] /= sum;
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k] += contrib;
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
    for (doc_id i{0}; i < idx_->num_docs(); ++i)
    {
        progress(i);
        for (auto& freq : idx_->search_primary(i)->counts())
        {
            // remove this word occurrence from means
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] -= contrib;
                topic_term_mean_[k][freq.first] -= contrib;
                topic_mean_[k] -= contrib;
            }
            double min = 0;
            double max = 0;
            std::unordered_map<topic_id, double> old_gammas =
                gamma_[i][freq.first];
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                // recompute gamma using CVB0 formula
                gamma_[i][freq.first][k] =
                    compute_term_topic_probability(freq.first, k) *
                    doc_topic_mean_.at(i).at(k);
                min = std::min(min, gamma_[i][freq.first][k]);
                max = std::max(max, gamma_[i][freq.first][k]);
            }
            // normalize gamma and update means
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma_[i][freq.first][k] =
                    (gamma_[i][freq.first][k] - min) / (max - min);
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k] += contrib;
                max_change =
                    std::max(max_change, std::abs(old_gammas[k] -
                                                  gamma_[i][freq.first][k]));
            }
        }
    }
    return max_change;
}

double lda_cvb::compute_term_topic_probability(term_id term,
                                               topic_id topic) const
{
    return (topic_term_mean_.at(topic).at(term) + beta_) /
           (topic_mean_.at(topic) + num_words_ * beta_);
}

double lda_cvb::compute_doc_topic_probability(doc_id doc, topic_id topic) const
{
    return (doc_topic_mean_.at(doc).at(topic) + alpha_) /
           (idx_->doc_size(doc) + num_topics_ * alpha_);
}
}
}
