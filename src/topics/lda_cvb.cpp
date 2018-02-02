/**
 * @file lda_cvb.cpp
 * @author Chase Geigle
 */

#include "meta/topics/lda_cvb.h"
#include "meta/logging/logger.h"
#include "meta/util/progress.h"
#include <random>

namespace meta
{
namespace topics
{

lda_cvb::lda_cvb(const learn::dataset& docs, std::size_t num_topics,
                 double alpha, double beta)
    : lda_model{docs, num_topics}
{
    gamma_.resize(docs_.size());

    // each theta_ is a multinomial over topics with a symmetric
    // Dirichlet(\alpha) prior
    theta_.reserve(docs_.size());
    for (const auto& doc : docs_)
    {
        theta_.emplace_back(stats::dirichlet<topic_id>{alpha, num_topics_});
        gamma_[doc.id].resize(doc_size(doc));
    }

    // each phi_ is a multinomial over terms with a symmetric
    // Dirichlet(\beta) prior
    phi_.reserve(num_topics_);
    for (topic_id topic{0}; topic < num_topics_; ++topic)
        phi_.emplace_back(
            stats::dirichlet<term_id>{beta, docs_.total_features()});
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
    printing::progress progress{"Initialization: ", docs_.size()};

    for (const auto& doc : docs_)
    {
        progress(doc.id);

        detail::update_gamma(doc.weights, num_topics_, gamma_[doc.id],
                             // decrease counts,
                             [](topic_id, term_id, double) {},
                             // update weight
                             [&](topic_id, term_id) { return rng(); },
                             // increase counts
                             [&](topic_id topic, term_id term, double prob) {
                                 theta_[doc.id].increment(topic, prob);
                                 phi_[topic].increment(term, prob);
                             });
    }
}

double lda_cvb::perform_iteration(uint64_t iter)
{
    printing::progress progress{"Iteration " + std::to_string(iter) + ": ",
                                docs_.size()};
    progress.print_endline(false);
    double max_change = 0;
    for (const auto& doc : docs_)
    {
        progress(doc.id);

        auto doc_max_change = detail::update_gamma(
            doc.weights, num_topics_, gamma_[doc.id],
            // decrease counts
            [&](topic_id topic, term_id term, double prob) {
                theta_[doc.id].decrement(topic, prob);
                phi_[topic].decrement(term, prob);
            },
            // update weight
            [&](topic_id topic, term_id term) {
                return theta_[doc.id].probability(topic)
                       * phi_[topic].probability(term);
            },
            // increase counts
            [&](topic_id topic, term_id term, double prob) {
                theta_[doc.id].increment(topic, prob);
                phi_[topic].increment(term, prob);
            });

        max_change = std::max(max_change, doc_max_change);
    }
    return max_change;
}

double lda_cvb::compute_term_topic_probability(term_id term,
                                               topic_id topic) const
{
    return phi_[topic].probability(term);
}

double lda_cvb::compute_doc_topic_probability(learn::instance_id doc,
                                              topic_id topic) const
{
    return theta_[doc].probability(topic);
}

stats::multinomial<topic_id> lda_cvb::topic_distribution(doc_id doc) const
{
    return theta_[doc];
}

stats::multinomial<term_id> lda_cvb::term_distribution(topic_id k) const
{
    return phi_[k];
}
}
}
