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

/**
 * Encapsulates the forward-backward algorithm using the scaling method
 * from the original Rabiner paper.
 *
 * @see http://www.ece.ucsb.edu/Faculty/Rabiner/ece259/Reprints/tutorial%20on%20hmm%20and%20applications.pdf
 * @see http://sifaka.cs.uiuc.edu/course/498cxz06s/hmm.pdf
 */
struct scaling_forward_backward
{
    template <class HMM>
    static util::dense_matrix<double>
    output_probabilities(const HMM& hmm, const typename HMM::sequence_type& seq)
    {
        const auto& obs_dist = hmm.observation_distribution();
        util::dense_matrix<double> output_probs{seq.size(), hmm.num_states()};

        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            for (state_id s_i{0}; s_i < hmm.num_states(); ++s_i)
            {
                output_probs(t, s_i) = obs_dist.probability(seq[t], s_i);
            }
        }
        return output_probs;
    }

    template <class HMM>
    static forward_trellis
    forward(const HMM& hmm, const typename HMM::sequence_type& seq,
            const util::dense_matrix<double>& output_probs)
    {
        forward_trellis fwd{seq.size(), hmm.num_states()};

        // initialize the first column of the trellis
        for (label_id l{0}; l < hmm.num_states(); ++l)
        {
            state_id s{l};
            fwd.probability(0, l, hmm.init_prob(s) * output_probs(0, s));
        }
        // normalize to avoid underflow
        fwd.normalize(0);

        // compute remaining columns using the recursive formulation
        for (uint64_t t = 1; t < seq.size(); ++t)
        {
            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};
                double sum = 0;
                for (label_id j{0}; j < hmm.num_states(); ++j)
                {
                    state_id s_j{j};
                    sum += fwd.probability(t - 1, j) * hmm.trans_prob(s_j, s_i);
                }
                fwd.probability(t, i, sum * output_probs(t, s_i));
            }
            // normalize to avoid underflow
            fwd.normalize(t);
        }

        return fwd;
    }

    template <class HMM>
    static trellis backward(const HMM& hmm,
                            const typename HMM::sequence_type& seq,
                            const forward_trellis& fwd,
                            const util::dense_matrix<double>& output_probs)
    {
        trellis bwd{seq.size(), hmm.num_states()};

        // initialize the last column of the trellis
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            bwd.probability(seq.size() - 1, i, 1);
        }

        // fill in the remaining columns of the trellis from back to front
        for (uint64_t k = 1; k < seq.size(); ++k)
        {
            assert(seq.size() - 1 >= k);
            uint64_t t = seq.size() - 1 - k;

            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};

                double sum = 0;
                for (label_id j{0}; j < hmm.num_states(); ++j)
                {
                    state_id s_j{j};

                    sum += bwd.probability(t + 1, j) * hmm.trans_prob(s_i, s_j)
                           * output_probs(t + 1, s_j);
                }
                auto norm = fwd.normalizer(t + 1);
                bwd.probability(t, i, norm * sum);
            }
        }

        return bwd;
    }

    template <class HMM>
    static util::dense_matrix<double>
    posterior_state_membership(const HMM& hmm, const forward_trellis& fwd,
                               const trellis& bwd)
    {
        util::dense_matrix<double> gamma{fwd.size(), hmm.num_states()};
        for (uint64_t t = 0; t < fwd.size(); ++t)
        {
            double norm = 0;
            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};
                gamma(t, s_i) = fwd.probability(t, i) * bwd.probability(t, i);
                norm += gamma(t, s_i);
            }
            std::transform(gamma.begin(t), gamma.end(t), gamma.begin(t),
                           [&](double val) { return val / norm; });
            // gamma(t, ) = prob. dist over possible states at time t
        }
        return gamma;
    }

    template <class HMM, class ExpectedCounts>
    static void increment_counts(const HMM& hmm, ExpectedCounts& counts,
                                 const typename HMM::sequence_type& seq,
                                 const forward_trellis& fwd, const trellis& bwd,
                                 const util::dense_matrix<double>& gamma,
                                 const util::dense_matrix<double>& output_probs)
    {
        // add expected counts to the new parameters
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            state_id s_i{i};

            // add expected counts for initial state probabilities
            counts.model_counts.increment_initial(s_i, gamma(0, s_i));

            // add expected counts for transition probabilities
            for (label_id j{0}; j < hmm.num_states(); ++j)
            {
                state_id s_j{j};

                for (uint64_t t = 0; t < seq.size() - 1; ++t)
                {
                    auto xi_tij
                        = (gamma(t, s_i) * hmm.trans_prob(s_i, s_j)
                           * output_probs(t + 1, s_j) * fwd.normalizer(t + 1)
                           * bwd.probability(t + 1, j))
                          / bwd.probability(t, i);

                    counts.model_counts.increment_transition(s_i, s_j, xi_tij);
                }
            }

            // add expected counts for observation probabilities
            for (uint64_t t = 0; t < seq.size(); ++t)
            {
                counts.obs_counts.increment(seq[t], s_i, gamma(t, s_i));
            }
        }

        // compute contribution to the log likelihood from the forward
        // trellis scaling factors for this sequence
        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            // L = \prod_o \prod_t 1 / scale(t)
            // log L = \sum_o \sum_t \log (1 / scale(t))
            // log L = \sum_o \sum_t - \log scale(t)
            counts.log_likelihood += -std::log(fwd.normalizer(t));
        }
    }
};

/**
 * Encapsulates the forward-backward algorithm using calculations in log
 * space. This is typically slower than the scaling method, but may be
 * necessary in some cases (like for observations that themselves are
 * sequences and have vanishingly small probabilities).
 */
struct logarithm_forward_backward
{
    template <class ForwardIterator>
    static double log_sum_exp(ForwardIterator begin, ForwardIterator end)
    {
        auto max_it = std::max_element(begin, end);

        auto shifted_sum_exp
            = std::accumulate(begin, end, 0.0, [=](double accum, double val) {
                  return accum + std::exp(val - *max_it);
              });

        return *max_it + std::log(shifted_sum_exp);
    }

    template <class HMM>
    static util::dense_matrix<double>
    output_probabilities(const HMM& hmm, const typename HMM::sequence_type& seq)
    {
        const auto& obs_dist = hmm.observation_distribution();
        util::dense_matrix<double> output_probs{seq.size(), hmm.num_states()};

        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            for (state_id s_i{0}; s_i < hmm.num_states(); ++s_i)
            {
                output_probs(t, s_i) = obs_dist.log_probability(seq[t], s_i);
            }
        }
        return output_probs;
    }

    template <class HMM>
    static trellis forward(const HMM& hmm,
                           const typename HMM::sequence_type& seq,
                           const util::dense_matrix<double>& output_log_probs)
    {
        trellis fwd{seq.size(), hmm.num_states()};

        // initialize the first column of the trellis
        for (label_id l{0}; l < hmm.num_states(); ++l)
        {
            state_id s{l};
            fwd.probability(0, l, std::log(hmm.init_prob(s))
                                      + output_log_probs(0, s));
        }

        std::vector<double> scratch(hmm.num_states());
        // compute remaining columns using the recursive formulation
        for (uint64_t t = 1; t < seq.size(); ++t)
        {
            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};
                for (label_id j{0}; j < hmm.num_states(); ++j)
                {
                    state_id s_j{j};
                    scratch[j] = fwd.probability(t - 1, j)
                                 + std::log(hmm.trans_prob(s_j, s_i));
                }
                fwd.probability(t, i,
                                log_sum_exp(scratch.begin(), scratch.end())
                                    + output_log_probs(t, s_i));
            }
        }

        return fwd;
    }

    template <class HMM>
    static trellis backward(const HMM& hmm,
                            const typename HMM::sequence_type& seq,
                            const trellis&,
                            const util::dense_matrix<double>& output_log_probs)
    {
        trellis bwd{seq.size(), hmm.num_states()};

        // initialize the last column of the trellis
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            bwd.probability(seq.size() - 1, i, 0);
        }

        std::vector<double> scratch(hmm.num_states());
        // fill in the remaining columns of the trellis from back to front
        for (uint64_t k = 1; k < seq.size(); ++k)
        {
            assert(seq.size() - 1 >= k);
            uint64_t t = seq.size() - 1 - k;

            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};
                for (label_id j{0}; j < hmm.num_states(); ++j)
                {
                    state_id s_j{j};
                    scratch[j] = bwd.probability(t + 1, j)
                                 + std::log(hmm.trans_prob(s_i, s_j))
                                 + output_log_probs(t + 1, s_j);
                }
                bwd.probability(t, i,
                                log_sum_exp(scratch.begin(), scratch.end()));
            }
        }

        return bwd;
    }

    template <class HMM>
    static util::dense_matrix<double>
    posterior_state_membership(const HMM& hmm, const trellis& fwd,
                               const trellis& bwd)
    {
        util::dense_matrix<double> gamma{fwd.size(), hmm.num_states()};
        std::vector<double> scratch(hmm.num_states());
        for (uint64_t t = 0; t < fwd.size(); ++t)
        {
            for (label_id i{0}; i < hmm.num_states(); ++i)
            {
                state_id s_i{i};
                gamma(t, s_i) = fwd.probability(t, i) + bwd.probability(t, i);
            }
            auto norm = log_sum_exp(gamma.begin(t), gamma.end(t));
            std::transform(gamma.begin(t), gamma.end(t), gamma.begin(t),
                           [=](double val) { return val - norm; });
        }
        return gamma;
    }

    template <class HMM, class ExpectedCounts>
    static void
    increment_counts(const HMM& hmm, ExpectedCounts& counts,
                     const typename HMM::sequence_type& seq, const trellis& fwd,
                     const trellis& bwd,
                     const util::dense_matrix<double>& log_gamma,
                     const util::dense_matrix<double>& output_log_probs)
    {
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            state_id s_i{i};

            // add expected counts for initial state probabilities
            counts.model_counts.increment_initial(s_i,
                                                  std::exp(log_gamma(0, s_i)));

            // add expected counts for transition probabilities
            for (label_id j{0}; j < hmm.num_states(); ++j)
            {
                state_id s_j{j};

                for (uint64_t t = 0; t < seq.size() - 1; ++t)
                {
                    auto log_xi_tij
                        = log_gamma(t, s_i) + std::log(hmm.trans_prob(s_i, s_j))
                          + output_log_probs(t + 1, s_j)
                          + bwd.probability(t + 1, j) - bwd.probability(t, i);

                    counts.model_counts.increment_transition(
                        s_i, s_j, std::exp(log_xi_tij));
                }
            }

            // add expected counts for observation probabilities
            for (uint64_t t = 0; t < seq.size(); ++t)
            {
                counts.obs_counts.increment(seq[t], s_i,
                                            std::exp(log_gamma(t, s_i)));
            }
        }

        // compute contribution to the log likelihood
        std::vector<double> scratch(hmm.num_states());
        for (label_id i{0}; i < hmm.num_states(); ++i)
        {
            scratch[i] = fwd.probability(seq.size() - 1, i);
        }
        counts.log_likelihood += log_sum_exp(scratch.begin(), scratch.end());
    }
};
}
}
}
#endif
