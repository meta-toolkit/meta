/**
 * @file topics/lda_gibbs_inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LDA_GIBBS_INFERENCER_H_
#define META_LDA_GIBBS_INFERENCER_H_

#include "meta/config.h"
#include "meta/topics/inferencer.h"
#include "meta/topics/lda_gibbs.h"

namespace meta
{
namespace topics
{

/**
 * An inferencer for topic proportions for unseen documents that uses
 * collapsed Gibbs sampling for inference.
 */
class lda_gibbs::inferencer : public meta::topics::inferencer
{
  public:
    using meta::topics::inferencer::inferencer;

    /**
     * Performs inference using collapsed Gibbs sampling to determine the
     * topic proportions for the supplied document. The topics themselves
     * are held fixed and are not modified by this function.
     *
     * @param doc the document to be analyzed
     * @param iters the number of iterations of sampling to apply
     * @param rng the random number generator to use during sampling
     */
    template <class RandomNumberGenerator>
    stats::multinomial<topic_id> operator()(const learn::feature_vector& doc,
                                            std::size_t iters,
                                            RandomNumberGenerator&& rng) const
    {
        auto doc_size = std::accumulate(
            doc.begin(), doc.end(), 0.0,
            [](double accum,
               const std::pair<learn::feature_id, double>& weight) {
                return accum + weight.second;
            });

        std::vector<topic_id> assignments(static_cast<std::size_t>(doc_size));
        stats::multinomial<topic_id> proportions{proportions_prior()};

        for (std::size_t i = 0; i < iters; ++i)
        {
            detail::sample_document(
                doc, num_topics(), assignments,
                // decrease counts
                [&](topic_id old_topic, term_id) {
                    if (i > 0)
                        proportions.decrement(old_topic, 1);
                },
                // sample weight
                [&](topic_id topic, term_id term) {
                    return proportions.probability(topic)
                           * term_distribution(topic).probability(term);
                },
                // increase counts
                [&](topic_id new_topic, term_id) {
                    proportions.increment(new_topic, 1);
                },
                std::forward<RandomNumberGenerator>(rng));
        }

        return proportions;
    }
};
}
}
#endif
