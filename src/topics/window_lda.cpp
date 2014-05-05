/**
 * @file window_lda.cpp
 * @author Chase Geigle
 */

#include <random>
#include "topics/window_lda.h"
#include "util/progress.h"
#include "topics/lda_model.h"

namespace meta
{
namespace topics
{

window_lda::window_lda(uint64_t num_topics, double alpha, double beta)
    : num_topics_{num_topics}, alpha_{alpha}, beta_{beta}
{
    std::random_device dev;
    rng_.seed(dev());
}

void window_lda::learn(const dataset& dset, uint64_t burn_in, uint64_t iters,
                       double convergence)
{
    // initialization step
    std::stringstream ss;
    ss << "Initialization: ";
    printing::progress progress{ss.str(), dset.size()};
    progress.print_endline(false);
    perform_iteration(dset, progress, 0);
    progress.clear();
    double likelihood = corpus_likelihood(dset);
    ss << "log likelihood: " << likelihood;
    LOG(progress) << '\r' << ss.str() << '\n' << ENDLG;

    // burn in iterations
    for (uint64_t i = 0; i < burn_in; ++i)
    {
        std::stringstream ss;
        ss << "Burn-in iteration " << i + 1 << ": ";
        printing::progress progress{ss.str(), dset.size()};
        progress.print_endline(false);
        perform_iteration(dset, progress, i + 1);
        progress.clear();
        double likelihood = corpus_likelihood(dset);
        ss << "log-likelihood: " << likelihood;
        LOG(progress) << '\r' << ss.str() << '\n' << ENDLG;
    }

    // actual sampling iterations
    for (uint64_t i = 0; i < iters; ++i)
    {
        std::stringstream ss;
        ss << "Iteration " << i + 1 << ": ";
        printing::progress progress{ss.str(), dset.size()};
        progress.print_endline(false);
        perform_iteration(dset, progress, i + 1);
        progress.clear();
        double likelihood_update = corpus_likelihood(dset);
        double ratio = std::fabs((likelihood - likelihood_update) / likelihood);
        likelihood = likelihood_update;
        ss << "log-likelihood: " << likelihood;
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

void window_lda::perform_iteration(const dataset& dset,
                                   printing::progress& progress, uint64_t iter)
{
    for (doc_id m{0}; m < dset.size(); ++m)
    {
        progress(m);
        const auto& seq = dset.at(m);
        for (segment_id n{0}; n < seq.size(); ++n)
        {
            auto old_topic = segment_topics_[m][n];

            // remove the counts for this assignment z_{m, n}
            if (iter > 0)
                decrease_counts(old_topic, m, seq[n]);

            // sample a new z_{m, n} from the full conditional
            auto topic = sample_topic(m, seq[n], dset.vocab_size());
            segment_topics_[m][n] = topic;

            // increase the counts for the new z_{m, n} we sampled
            increase_counts(topic, m, seq[n]);
        }
    }
}

void window_lda::decrease_counts(topic_id topic, doc_id doc,
                                 const sequence::observation& window)
{
    // size of the window == number of word occurrences, not number of
    // unique words
    double size = 0;
    for (const auto& p : window.features())
    {
        size += p.second;
        delta_(p.first, topic) -= p.second;
    }

    sigma_(doc, topic) -= 1;
    topic_count_[topic] -= size;
}

void window_lda::increase_counts(topic_id topic, doc_id doc,
                                 const sequence::observation& window)
{
    // size of the window == number of word occurrences, not number of
    // unique words
    double size = 0;
    for (const auto& p : window.features())
    {
        size += p.second;
        delta_(p.first, topic) += p.second;
    }

    sigma_(doc, topic) += 1;
    topic_count_[topic] += size;
}

topic_id window_lda::sample_topic(doc_id doc,
                                  const sequence::observation& window,
                                  uint64_t vocab_size)
{
    std::vector<double> weights(num_topics_);
    for (topic_id k{0}; k < num_topics_; ++k)
    {
        auto topic_contrib = sigma_(doc, k) + alpha_;

        double word_contrib = 1;
        uint64_t size = 0;
        for (const auto& p : window.features())
        {
            const auto& r = p.first;
            const auto& tau = p.second;
            size += tau;
            for (uint64_t t = 0; t < tau; ++t)
            {
                word_contrib *= (delta_(r, k) + beta_ + t);
            }
        }
        for (uint64_t t = 0; t < size; ++t)
        {
            word_contrib /= (t + vocab_size * beta_ + topic_count_[k]);
        }
        weights[k] = topic_contrib * word_contrib;
    }
    std::discrete_distribution<uint64_t> dist(weights.begin(), weights.end());
    return topic_id{dist(rng_)};
}

double window_lda::corpus_likelihood(const dataset& dset) const
{
    auto likelihood = num_topics_ * (std::lgamma(dset.vocab_size() * beta_)
                                     - dset.vocab_size() * std::lgamma(beta_));
    for (topic_id i{0}; i < num_topics_; ++i)
    {
        for (term_id r{0}; r < dset.vocab_size(); ++r)
        {
            likelihood += std::lgamma(delta_(r, i) + beta_);
        }
        likelihood -= std::lgamma(topic_count_[i] + dset.vocab_size() * beta_);
    }
    return likelihood;
}

void window_lda::save(const std::string& prefix, const dataset& dset) const
{
    save_doc_topic_distributions(prefix + ".theta");
    save_topic_term_distributions(prefix + ".phi", dset);
    save_segments(prefix + ".segments", dset);

    dset.save_vocabulary(prefix + ".vocab");
}

void window_lda::save_doc_topic_distributions(const std::string& filename) const
{
    std::ofstream file{filename};
    for (doc_id m{0}; m < sigma_.rows(); ++m)
    {
        file << m << "\t";
        double sum = 0;
        for (topic_id k{0}; k < num_topics_; ++k)
        {
            sum += sigma_(m, k) + alpha_;
        }
        for (topic_id k{0}; k < num_topics_; ++k)
        {
            auto prob = (sigma_(m, k) + alpha_) / sum;
            if (prob > 0)
                file << k << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void window_lda::save_topic_term_distributions(const std::string& filename,
                                               const dataset& dset) const
{
    std::ofstream file{filename};
    for (topic_id k{0}; k < num_topics_; ++k)
    {
        file << k << "\t";
        for (term_id r{0}; r < dset.vocab_size(); ++r)
        {
            auto prob = (delta_(r, k) + beta_)
                        / (topic_count_[k] + dset.vocab_size() * beta_);
            file << r << ":" << prob << "\t";
        }
        file << "\n";
    }
}

void window_lda::save_segments(const std::string& filename,
                               const dataset& dset) const
{
    std::ofstream file{filename};
    for (doc_id m{0}; m < dset.size(); ++m)
    {
        const auto& seq = dset.at(m);
        for (uint64_t x = 0; x < seq.size(); ++x)
        {
            file << segment_topics_[m][x] << "\t";
            for (const auto& p : seq[x].features())
            {
                file << p.first << ":" << p.second << "\t";
            }
            file << "\n";
        }
        file << "\n";
    }
}

}
}
