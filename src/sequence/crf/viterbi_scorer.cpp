/**
 * @file viterbi_scorer.cpp
 * @author Chase Geigle
 */

#include "meta/sequence/crf/viterbi_scorer.h"

namespace meta
{
namespace sequence
{

crf::viterbi_scorer::viterbi_scorer(const crf& model)
    : model_{&model}
{
    // these only ever need computing once because the underlying model is
    // not changing
    scorer_.transition_scores(*model_);
}

viterbi_trellis crf::viterbi_scorer::viterbi(const sequence& seq)
{
    // we only need the scores for the states as the transition scores, set
    // up during construction, will never change between sequences
    scorer_.state_scores(*model_, seq);

    viterbi_trellis table{seq.size(), model_->num_labels()};

    // initialize first column of trellis. We use the original state() and
    // trans() matrices because we are working in the log domain.
    for (label_id lbl{0}; lbl < model_->num_labels(); ++lbl)
        table.probability(0, lbl, scorer_.state(0, lbl));

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        for (label_id lbl{0}; lbl < model_->num_labels(); ++lbl)
        {
            double max_score = std::numeric_limits<double>::lowest();
            for (label_id in{0}; in < model_->num_labels(); ++in)
            {
                auto score = table.probability(t - 1, in)
                             + scorer_.trans(in, lbl);

                if (score > max_score)
                {
                    max_score = score;
                    table.previous_tag(t, lbl, in);
                }
            }
            table.probability(t, lbl, max_score + scorer_.state(t, lbl));
        }
    }
    return table;
}

}
}
