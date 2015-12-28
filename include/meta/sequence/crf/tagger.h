/**
 * @file tagger.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_CRF_TAGGER_H_
#define META_SEQUENCE_CRF_TAGGER_H_

#include "meta/sequence/crf/viterbi_scorer.h"

namespace meta
{
namespace sequence
{

class crf::tagger
{
  public:
    /**
     * Constructs a tagger against the given model.
     * @param model The model to use for the tagging
     */
    tagger(const crf& model);

    /**
     * Tags a sequence. The tags will be filled in on the `label` field
     * of each observation within the sequence. (You will need to ask
     * your analyzer for what the human-readable `tag_t` is for each
     * `label_id` yourself.)
     * @param seq The sequence to be tagged.
     */
    void tag(sequence& seq);

  private:
    class impl;
    /// the scorer used internally to run viterbi
    crf::viterbi_scorer scorer_;
    /// the number of labels
    uint64_t num_labels_;
};

}
}
#endif
