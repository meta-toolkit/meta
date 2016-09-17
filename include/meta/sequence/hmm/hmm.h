/**
 * @file hmm.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_HMM_H_
#define META_SEQUENCE_HMM_H_

#include <cassert>

#include "meta/config.h"
#include "meta/logging/logger.h"
#include "meta/sequence/trellis.h"
#include "meta/stats/multinomial.h"
#include "meta/util/identifiers.h"
#include "meta/util/progress.h"
#include "meta/util/random.h"
#include "meta/util/time.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

MAKE_NUMERIC_IDENTIFIER(state_id, uint64_t)

class hmm_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * A generic Hidden Markov Model implementation for unsupervised sequence
 * labeling tasks.
 */
template <class ObsDist>
class hidden_markov_model
{
  public:
    using observation_type = typename ObsDist::observation_type;
    using sequence_type = std::vector<observation_type>;
    using training_data_type = std::vector<sequence_type>;

    struct training_options
    {
        /**
         * The convergence threshold. When the difference in log likelihood
         * between iterations falls below this value, training will stop.
         */
        double delta = 1e-5;

        /**
         * The maximum number of iterations. If the difference in log
         * likelihood has not reached the convergence threshold after this
         * many iterations, stop training.
         */
        uint64_t max_iters = std::numeric_limits<uint64_t>::max();
    };

    /**
     * Constructs a new Hidden Markov Model with random initialization
     * using the provided random number generator. The observation
     * distribution must be provided and is not initialized by the
     * constructor (so you should initialize it yourself using an
     * appropriate constructor for it).
     *
     * @param num_states The number of hidden states in the HMM
     * @param gen The random number generator to use for initialization
     * @param obs_dist The observation distribution
     * @param trans_prior The Dirichlet prior over the transitions
     */
    template <class Generator>
    hidden_markov_model(uint64_t num_states, Generator&& rng,
                        ObsDist&& obs_dist,
                        stats::dirichlet<state_id>&& trans_prior)
        : obs_dist_{std::move(obs_dist)}, trans_dists_(num_states, trans_prior)
    {
        if (obs_dist_.num_states() != num_states)
            throw hmm_exception{"The observation distribution and HMM have "
                                "differing numbers of hidden states"};

        for (auto& trans_dist : trans_dists_)
        {
            for (state_id s_i{0}; s_i < num_states; ++s_i)
            {
                auto rnd = random::bounded_rand(rng, 65536);
                auto val = (rnd / 65536.0) / num_states;
                trans_dist.increment(s_i, val);
            }
        }

        for (state_id s_i{0}; s_i < num_states; ++s_i)
        {
            auto rnd = random::bounded_rand(rng, 65536);
            auto val = (rnd / 65536.0) / num_states;
            initial_dist_.increment(s_i, val);
        }
    }

    /**
     * Constructs a new Hidden Markov Model with uniform initialization of
     * initial state and transition distributions. The observation
     * distribution must be provided and is not initialized by the
     * constructor (so you should initialize it yourself using an
     * appropriate constructor for it). The initialization of the
     * observation distribution is quite important as this is the only
     * distribution that distinguishes states from one another when this
     * constructor is used, so it is recommended to use a random
     * initialization for it if possible.
     *
     * @param num_states The number of hidden states in the HMM
     * @param obs_dist The observation distribution
     * @param trans_prior The Dirichlet prior over the transitions
     */
    hidden_markov_model(uint64_t num_states, ObsDist&& obs_dist,
                        stats::dirichlet<state_id>&& trans_prior)
        : obs_dist_{std::move(obs_dist)}, trans_dists_(num_states, trans_prior)
    {
        if (obs_dist_.num_states() != num_states)
            throw hmm_exception{"The observation distribution and HMM have "
                                "differing numbers of hidden states"};

        for (auto& trans_dist : trans_dists_)
        {
            for (state_id s_i{0}; s_i < num_states; ++s_i)
            {
                trans_dist.increment(s_i, 1.0);
            }
        }

        for (state_id s_i{0}; s_i < num_states; ++s_i)
        {
            initial_dist_.increment(s_i, 1.0);
        }
    }

    /**
     * @param instances The training data to fit the model to
     * @param options The training options
     * @return the log likelihood of the data
     */
    double fit(const training_data_type& instances, training_options options)
    {
        double old_ll = std::numeric_limits<double>::lowest();
        for (uint64_t iter = 1; iter <= options.max_iters; ++iter)
        {
            double ll = 0;

            auto time = common::time([&]() {
                printing::progress progress{"> Iteration "
                                                + std::to_string(iter) + ": ",
                                            instances.size()};
                ll = expectation_maximization(instances, progress);
            });

            LOG(info) << "Took " << time.count() / 1000.0 << "s" << ENDLG;
            LOG(info) << "Log likelihood: " << ll << ENDLG;

            if (old_ll > ll)
            {
                LOG(fatal) << "Log likelihood did not improve!" << ENDLG;
                throw std::runtime_error{"Log likelihood did not improve"};
            }

            if (ll - old_ll < options.delta)
            {
                LOG(info) << "Converged! (" << ll - old_ll << " < "
                          << options.delta << ")" << ENDLG;
                return ll;
            }

            old_ll = ll;
        }
        return old_ll;
    }

    uint64_t num_states() const
    {
        return trans_dists_.size();
    }

    double trans_prob(state_id from, state_id to) const
    {
        return trans_dists_[from].probability(to);
    }

  private:
    double expectation_maximization(const training_data_type& instances,
                                    printing::progress& progress)
    {
        // allocate space for the new parameters
        auto new_obs_dist = obs_dist_.blank();

        std::vector<stats::multinomial<state_id>> new_trans_dists;
        new_trans_dists.reserve(num_states());
        for (const auto& tdist : trans_dists_)
            new_trans_dists.emplace_back(tdist.prior());

        stats::multinomial<state_id> new_initial_dist{initial_dist_.prior()};

        // compute expected counts across all instances
        double log_likelihood = 0;
        uint64_t seq_id = 0;
        for (const auto& seq : instances)
        {
            progress(seq_id++);

            // cache b_i(o_t) since this could be computed with an
            // arbitrarily complex model
            auto output_probs = output_probabilities(seq);

            // run forward-backward to get the trellises
            auto fwd = forward(seq, output_probs);
            auto bwd = backward(seq, fwd, output_probs);

            // compute the probability of being in a given state at a given
            // time from the trellises
            auto gamma = posterior_state_membership(fwd, bwd);

            // add expected counts to the new parameters
            for (label_id i{0}; i < num_states(); ++i)
            {
                state_id s_i{i};

                // add expected counts for initial state probabilities
                new_initial_dist.increment(s_i, gamma[0].probability(s_i));

                // add expected counts for transition probabilities
                for (label_id j{0}; j < num_states(); ++j)
                {
                    state_id s_j{j};

                    for (uint64_t t = 0; t < seq.size() - 1; ++t)
                    {
                        auto xi_tij
                            = (gamma[t].probability(s_i) * trans_prob(s_i, s_j)
                               * output_probs(t + 1, s_j)
                               * fwd.normalizer(t + 1)
                               * bwd.probability(t + 1, j))
                              / bwd.probability(t, i);

                        new_trans_dists[s_i].increment(s_j, xi_tij);
                    }
                }

                // add expected counts for observation probabilities
                for (uint64_t t = 0; t < seq.size(); ++t)
                {
                    new_obs_dist.increment(seq[t], s_i,
                                           gamma[t].probability(s_i));
                }
            }

            // compute contribution to the log likelihood from the forward
            // trellis scaling factors for this sequence
            for (uint64_t t = 0; t < seq.size(); ++t)
            {
                // L = \prod_o \prod_t 1 / scale(t)
                // log L = \sum_o \sum_t \log (1 / scale(t))
                // log L = \sum_o \sum_t - \log scale(t)
                log_likelihood += -std::log(fwd.normalizer(t));
            }
        }

        // replace old parameters
        obs_dist_ = std::move(new_obs_dist);
        trans_dists_ = std::move(new_trans_dists);
        initial_dist_ = std::move(new_initial_dist);

        return log_likelihood;
    }

    util::dense_matrix<double>
    output_probabilities(const sequence_type& seq) const
    {
        util::dense_matrix<double> output_probs{seq.size(), num_states()};

        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            for (state_id s_i{0}; s_i < num_states(); ++s_i)
            {
                output_probs(t, s_i) = obs_dist_.probability(seq[t], s_i);
            }
        }
        return output_probs;
    }

    std::vector<stats::multinomial<state_id>>
    posterior_state_membership(const forward_trellis& fwd, const trellis& bwd)
    {
        std::vector<stats::multinomial<state_id>> gamma(fwd.size());

        for (uint64_t t = 0; t < fwd.size(); ++t)
        {
            for (label_id i{0}; i < num_states(); ++i)
            {
                state_id s_i{i};
                gamma[t].increment(s_i, fwd.probability(t, i)
                                            * bwd.probability(t, i));
            }
            // gamma[t] = prob. dist over possible states at time t
        }
        return gamma;
    }

    forward_trellis
    forward(const sequence_type& seq,
            const util::dense_matrix<double>& output_probs) const
    {
        forward_trellis fwd{seq.size(), num_states()};

        // initialize the first column of the trellis
        for (label_id l{0}; l < num_states(); ++l)
        {
            state_id s{l};
            fwd.probability(0, l,
                            initial_dist_.probability(s) * output_probs(0, s));
        }
        // normalize to avoid underflow
        fwd.normalize(0);

        // compute remaining columns using the recursive formulation
        for (uint64_t t = 1; t < seq.size(); ++t)
        {
            for (label_id i{0}; i < num_states(); ++i)
            {
                state_id s_i{i};
                double sum = 0;
                for (label_id j{0}; j < num_states(); ++j)
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

    trellis backward(const sequence_type& seq, const forward_trellis& fwd,
                     const util::dense_matrix<double>& output_probs) const
    {
        trellis bwd{seq.size(), num_states()};

        // initialize the last column of the trellis
        for (label_id i{0}; i < num_states(); ++i)
        {
            bwd.probability(seq.size() - 1, i, 1);
        }

        // fill in the remaining columns of the trellis from back to front
        for (uint64_t k = 1; k < seq.size(); ++k)
        {
            assert(seq.size() - 1 >= k);
            uint64_t t = seq.size() - 1 - k;

            for (label_id i{0}; i < num_states(); ++i)
            {
                state_id s_i{i};

                double sum = 0;
                for (label_id j{0}; j < num_states(); ++j)
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

    ObsDist obs_dist_;
    std::vector<stats::multinomial<state_id>> trans_dists_;
    stats::multinomial<state_id> initial_dist_;
};
}
}
}
#endif
