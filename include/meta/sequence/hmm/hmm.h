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
#include "meta/parallel/algorithm.h"
#include "meta/sequence/hmm/forward_backward.h"
#include "meta/sequence/markov_model.h"
#include "meta/sequence/trellis.h"
#include "meta/stats/multinomial.h"
#include "meta/util/identifiers.h"
#include "meta/util/progress.h"
#include "meta/util/random.h"
#include "meta/util/time.h"
#include "meta/util/traits.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

class hmm_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

template <class ObsDist>
struct hmm_traits
{
    using observation_type = typename ObsDist::observation_type;
    using sequence_type = std::vector<observation_type>;
    using training_data_type = std::vector<sequence_type>;
    using forward_backward_type = scaling_forward_backward;
};

/**
 * A generic Hidden Markov Model implementation for unsupervised sequence
 * labeling tasks.
 */
template <class ObsDist>
class hidden_markov_model
{
  public:
    using traits_type = hmm_traits<ObsDist>;
    using observation_type = typename traits_type::observation_type;
    using sequence_type = typename traits_type::sequence_type;
    using training_data_type = typename traits_type::training_data_type;
    using forward_backward_type = typename traits_type::forward_backward_type;

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
                        stats::dirichlet<state_id> trans_prior)
        : obs_dist_{std::move(obs_dist)}, model_{num_states, rng, trans_prior}
    {
        if (obs_dist_.num_states() != num_states)
            throw hmm_exception{"The observation distribution and HMM have "
                                "differing numbers of hidden states"};
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
                        stats::dirichlet<state_id> trans_prior)
        : obs_dist_{std::move(obs_dist)}, model_{num_states, trans_prior}
    {
        if (obs_dist_.num_states() != num_states)
            throw hmm_exception{"The observation distribution and HMM have "
                                "differing numbers of hidden states"};
    }

    /**
     * Loads a hidden Markov model from an input stream.
     */
    template <class InputStream,
              class = util::disable_if_same_or_derived_t<hidden_markov_model,
                                                         InputStream>>
    hidden_markov_model(InputStream& is) : obs_dist_{is}, model_{is}
    {
        // nothing
    }

    /**
     * @param instances The training data to fit the model to
     * @param options The training options
     * @return the log likelihood of the data
     */
    double fit(const training_data_type& instances, parallel::thread_pool& pool,
               training_options options)
    {
        double old_ll = std::numeric_limits<double>::lowest();
        for (uint64_t iter = 1; iter <= options.max_iters; ++iter)
        {
            double log_likelihood = 0;

            auto em_time = common::time([&]() {
                printing::progress progress{"> Iteration "
                                                + std::to_string(iter) + ": ",
                                            instances.size()};
                log_likelihood
                    = expectation_maximization(instances, pool, progress);
            });

            auto relative_change = (old_ll - log_likelihood) / old_ll;
            LOG(info) << "Took " << em_time.count() / 1000.0 << "s" << ENDLG;

            if (iter > 1)
            {
                LOG(info) << "Log likelihood: " << log_likelihood << " (+"
                          << relative_change << " relative change)" << ENDLG;
            }
            else
            {
                LOG(info) << "Log log_likelihood: " << log_likelihood << ENDLG;
            }

            assert(old_ll <= log_likelihood);

            if (iter > 1 && relative_change < options.delta)
            {
                LOG(info) << "Converged! (" << relative_change << " < "
                          << options.delta << ")" << ENDLG;
                return log_likelihood;
            }

            old_ll = log_likelihood;
        }

        return old_ll;
    }

    uint64_t num_states() const
    {
        return model_.num_states();
    }

    double trans_prob(state_id from, state_id to) const
    {
        return model_.transition_probability(from, to);
    }

    double init_prob(state_id s) const
    {
        return model_.initial_probability(s);
    }

    const ObsDist& observation_distribution() const
    {
        return obs_dist_;
    }

    const typename ObsDist::conditional_distribution_type&
    observation_distribution(state_id s) const
    {
        return obs_dist_.distribution(s);
    }

    template <class OutputStream>
    void save(OutputStream& os) const
    {
        obs_dist_.save(os);
        model_.save(os);
    }

    /**
     * Temporary storage for expected counts for the different model types,
     * plus the data log likelihood computed during the forward-backward
     * algorithm
    */
    struct expected_counts
    {
        expected_counts(const hidden_markov_model& hmm)
            : obs_counts{hmm.obs_dist_.expected_counts()},
              model_counts{hmm.model_.expected_counts()}
        {
            // nothing
        }

        expected_counts& operator+=(const expected_counts& other)
        {
            obs_counts += other.obs_counts;
            model_counts += other.model_counts;
            log_likelihood += other.log_likelihood;
            return *this;
        }

        typename ObsDist::expected_counts_type obs_counts;
        markov_model::expected_counts_type model_counts;
        double log_likelihood = 0.0;
    };

    /**
     * Computes expected counts using the forward-backward algorithm.
     */
    expected_counts forward_backward(const sequence_type& seq)
    {
        expected_counts ec{*this};
        forward_backward(seq, ec);
        return ec;
    }

  private:
    void forward_backward(const sequence_type& seq, expected_counts& counts)
    {
        using fwdbwd = forward_backward_type;
        // cache b_i(o_t) since this could be computed with an
        // arbitrarily complex model
        auto output_probs = fwdbwd::output_probabilities(*this, seq);

        // run forward-backward
        auto fwd = fwdbwd::forward(*this, seq, output_probs);
        auto bwd = fwdbwd::backward(*this, seq, fwd, output_probs);

        // compute the probability of being in a given state at a given
        // time from the trellises
        auto gamma = fwdbwd::posterior_state_membership(*this, fwd, bwd);

        // increment expected counts
        fwdbwd::increment_counts(*this, counts, seq, fwd, bwd, gamma,
                                 output_probs);
    }

    double expectation_maximization(const training_data_type& instances,
                                    parallel::thread_pool& pool,
                                    printing::progress& progress)
    {
        uint64_t seq_id = 0;
        // compute expected counts across all instances in parallel
        std::mutex progress_mutex;
        auto counts = parallel::reduction(
            instances.begin(), instances.end(), pool,
            [&]() { return expected_counts{*this}; },
            [&](expected_counts& counts, const sequence_type& seq) {
                {
                    std::lock_guard<std::mutex> lock{progress_mutex};
                    progress(seq_id++);
                }
                forward_backward(seq, counts);
            },
            [&](expected_counts& result, const expected_counts& temp) {
                result += temp;
            });

        // normalize and replace old parameters
        obs_dist_ = ObsDist{std::move(counts.obs_counts)};
        model_ = markov_model{std::move(counts.model_counts)};

        return counts.log_likelihood;
    }

    ObsDist obs_dist_;
    markov_model model_;
};
}
}
}
#endif
