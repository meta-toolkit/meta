/**
 * @file viterbi_scorer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_CRF_VITERBI_SCORER_H_
#define META_SEQUENCE_CRF_VITERBI_SCORER_H_

#include "meta/sequence/crf/scorer.h"

namespace meta
{
namespace sequence
{

/**
 * Scorer for performing viterbi-based tagging.
 */
class crf::viterbi_scorer
{
  public:
    /**
     * Constructs a new scorer against the given model.
     * @param model The model to score with
     */
    viterbi_scorer(const crf& model);

    /**
     * Runs the viterbi algorithm to produce a trellis with
     * back-pointers.
     *
     * @param seq The sequence to score
     * @return a trellis with back-pointers indicating the path with
     * the highest score
     */
    viterbi_trellis viterbi(const sequence& seq);

  private:
    /// the internal scorer used
    crf::scorer scorer_;
    /// a back-pointer to the model this scorer uses to tag
    const crf* model_;
};
}
}
#endif
