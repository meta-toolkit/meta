/**
 * @file tagger.cpp
 * @author Chase Geigle
 */

#include "meta/sequence/crf/tagger.h"
#include "meta/util/functional.h"

namespace meta
{
namespace sequence
{

auto crf::make_tagger() const -> tagger
{
    return tagger{*this};
}

crf::tagger::tagger(const crf& model)
    : scorer_{model}, num_labels_{model.num_labels()}
{
    // nothing
}

void crf::tagger::tag(sequence& seq)
{
    auto trellis = scorer_.viterbi(seq);

    auto lbls = util::range(label_id{0},
                            label_id(static_cast<uint32_t>(num_labels_ - 1)));
    auto last_lbl = functional::argmax(
        lbls.begin(), lbls.end(), [&](label_id lbl)
        {
            return trellis.probability(seq.size() - 1, lbl);
        });

    seq[seq.size() - 1].label(*last_lbl);
    for (uint64_t t = seq.size() - 1; t > 0; t--)
        seq[t - 1].label(trellis.previous_tag(t, seq[t].label()));
}
}
}
