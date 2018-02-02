/**
 * @file topics/lda_cvb_inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/topics/lda_cvb_inferencer.h"

namespace meta
{
namespace topics
{
stats::multinomial<topic_id> lda_cvb::inferencer::
operator()(const learn::feature_vector& doc, std::size_t max_iters,
           double convergence) const

{
    auto doc_size = std::accumulate(
        doc.begin(), doc.end(), 0.0,
        [](double accum, const std::pair<learn::feature_id, double>& weight) {
            return accum + weight.second;
        });

    std::vector<stats::multinomial<topic_id>> gammas(
        static_cast<std::size_t>(doc_size));
    stats::multinomial<topic_id> proportions{proportions_prior()};

    for (std::size_t i = 0; i < max_iters; ++i)
    {
        auto max_change = detail::update_gamma(
            doc, num_topics(), gammas,
            // decrease counts
            [&](topic_id topic, term_id, double prob) {
                if (i > 0)
                    proportions.decrement(topic, prob);
            },
            // update weight
            [&](topic_id topic, term_id term) {
                return proportions.probability(topic)
                       * term_distribution(topic).probability(term);
            },
            // increase counts
            [&](topic_id topic, term_id, double prob) {
                proportions.increment(topic, prob);
            });

        if (max_change < convergence)
            break;
    }

    return proportions;
}
}
}
