/**
 * @file forward_backward.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_HMM_FORWARD_BACKWARD_H_
#define META_SEQUENCE_HMM_FORWARD_BACKWARD_H_

#include "meta/config.h"
#include "meta/sequence/markov_model.h"
#include "meta/sequence/trellis.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

template <class SequenceType>
struct scaling_forward_backward
{
    using sequence_type = SequenceType;

    template <class InitProb, class TransProb>
    static forward_trellis
    forward(const sequence_type& seq,
            const util::dense_matrix<double>& output_probs, uint64_t num_states,
            InitProb&& init_prob, TransProb&& trans_prob)
    {
        forward_trellis fwd{seq.size(), num_states};

        // initialize the first column of the trellis
        for (label_id l{0}; l < num_states; ++l)
        {
            state_id s{l};
            fwd.probability(0, l, init_prob(s) * output_probs(0, s));
        }
        // normalize to avoid underflow
        fwd.normalize(0);

        // compute remaining columns using the recursive formulation
        for (uint64_t t = 1; t < seq.size(); ++t)
        {
            for (label_id i{0}; i < num_states; ++i)
            {
                state_id s_i{i};
                double sum = 0;
                for (label_id j{0}; j < num_states; ++j)
                {
                    state_id s_j{j};
                    sum += fwd.probability(t - 1, j) * trans_prob(s_j, s_i);
                }
                fwd.probability(t, i, sum * output_probs(t, s_i));
            }
            // normalize to avoid underflow
            fwd.normalize(t);
        }

        return fwd;
    }

    template <class TransProb>
    static trellis backward(const sequence_type& seq,
                            const forward_trellis& fwd,
                            const util::dense_matrix<double>& output_probs,
                            uint64_t num_states, TransProb&& trans_prob)
    {
        trellis bwd{seq.size(), num_states};

        // initialize the last column of the trellis
        for (label_id i{0}; i < num_states; ++i)
        {
            bwd.probability(seq.size() - 1, i, 1);
        }

        // fill in the remaining columns of the trellis from back to front
        for (uint64_t k = 1; k < seq.size(); ++k)
        {
            assert(seq.size() - 1 >= k);
            uint64_t t = seq.size() - 1 - k;

            for (label_id i{0}; i < num_states; ++i)
            {
                state_id s_i{i};

                double sum = 0;
                for (label_id j{0}; j < num_states; ++j)
                {
                    state_id s_j{j};

                    sum += bwd.probability(t + 1, j) * trans_prob(s_i, s_j)
                           * output_probs(t + 1, s_j);
                }
                auto norm = fwd.normalizer(t + 1);
                bwd.probability(t, i, norm * sum);
            }
        }

        return bwd;
    }
};

template <class SequenceType>
struct logarithm_forward_backward
{
};
}
}
}
#endif
