/**
 * @file topics/lda_cvb_inferencer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LDA_CVB_INFERENCER_H_
#define META_LDA_CVB_INFERENCER_H_

#include "meta/config.h"
#include "meta/topics/inferencer.h"
#include "meta/topics/lda_cvb.h"

namespace meta
{
namespace topics
{

/**
 * An inferencer for topic proportions for unseen documents that uses
 * collapsed variational Bayes inference.
 */
class lda_cvb::inferencer : public meta::topics::inferencer
{
  public:
    using meta::topics::inferencer::inferencer;

    /**
     * Performs inference using the CVB0 algorithm to determine the topic
     * proportions for the supplied document. The topics themselves are
     * held fixed and are not modified by this function.
     *
     * @param doc the document to be analyzed
     * @param max_iters the maximum number of inference iterations
     * @param convergence convergence threshold (maximum L1 change in
     * word-topic distribution vectors)
     */
    stats::multinomial<topic_id> operator()(const learn::feature_vector& doc,
                                            std::size_t max_iters,
                                            double convergence) const;
};
}
}
#endif
