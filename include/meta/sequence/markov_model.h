/**
 * @file markov_model.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_MARKOV_MODEL_H_
#define META_SEQUENCE_MARKOV_MODEL_H_

#include "meta/stats/dirichlet.h"
#include "meta/util/dense_matrix.h"
#include "meta/util/identifiers.h"
#include "meta/util/random.h"
#include "meta/util/traits.h"

namespace meta
{
namespace sequence
{

MAKE_NUMERIC_IDENTIFIER(state_id, uint64_t)

/**
 * Represents a Markov model over a set of states.
 */
class markov_model
{
  public:
    /**
     * Represents expected counts for re-estimating a markov_model.
     */
    class expected_counts_type
    {
      public:
        friend markov_model;

        expected_counts_type(uint64_t num_states,
                             stats::dirichlet<state_id> prior);

        void increment(const std::vector<state_id>& seq, double amount);
        void increment_initial(state_id s, double amount);
        void increment_transition(state_id from, state_id to, double amount);

        expected_counts_type& operator+=(const expected_counts_type& other);

      private:
        std::vector<double> initial_count_;
        util::dense_matrix<double> trans_count_;
        stats::dirichlet<state_id> prior_;
    };

    /**
     * Constructs a new Markov Model with random initialization using the
     * provided random number generator.
     */
    template <class Generator>
    markov_model(uint64_t num_states, Generator&& rng,
                 stats::dirichlet<state_id> prior)
        : initial_prob_(num_states),
          trans_prob_{num_states, num_states},
          prior_{std::move(prior)}
    {
        double inorm = 0;
        for (state_id s_i{0}; s_i < num_states; ++s_i)
        {
            auto rnd = random::bounded_rand(rng, 65536);
            auto val = (rnd / 65536.0) / num_states;
            initial_prob_[s_i] = val;
            inorm += val;

            double tnorm = 0;
            for (state_id s_j{0}; s_j < num_states; ++s_j)
            {
                auto rnd = random::bounded_rand(rng, 65536);
                auto val = (rnd / 65536.0) / num_states;
                trans_prob_(s_i, s_j) = val;
                tnorm += val;
            }
            for (state_id s_j{0}; s_j < num_states; ++s_j)
            {
                trans_prob_(s_i, s_j)
                    = (trans_prob_(s_i, s_j) + prior_.pseudo_counts(s_j))
                      / (tnorm + prior_.pseudo_counts());
            }
        }

        for (state_id s_i{0}; s_i < num_states; ++s_i)
        {
            initial_prob_[s_i]
                = (initial_prob_[s_i] + prior_.pseudo_counts(s_i))
                  / (inorm + prior_.pseudo_counts());
        }
    }

    /**
     * Loads a Markov model from a file.
     */
    template <class InputStream,
              class = util::disable_if_same_or_derived_t<markov_model,
                                                         InputStream>>
    markov_model(InputStream& is)
    {
        if (io::packed::read(is, initial_prob_) == 0)
            throw std::runtime_error{"failed to read markov model from stream"};
        if (io::packed::read(is, trans_prob_) == 0)
            throw std::runtime_error{"failed to read markov model from stream"};
        if (io::packed::read(is, prior_) == 0)
            throw std::runtime_error{"failed to read markov model from stream"};
    }

    /**
     * Constructs a new Markov model with uniform initialization of
     * initial state and transition distibutions.
     */
    markov_model(uint64_t num_states, stats::dirichlet<state_id> prior);

    /**
     * Constructs a new Markov model from a set of expected counts.
     */
    markov_model(expected_counts_type&& counts);

    /**
     * Obtains an expected_counts_type suitable for re-estimating this
     * Markov model.
     */
    expected_counts_type expected_counts() const;

    /**
     * Obtains a reference to the prior used for the model.
     */
    const stats::dirichlet<state_id>& prior() const;

    /**
     * @return the number of states in the Markov model
     */
    uint64_t num_states() const;

    /**
     * @return \f$\log P(\mathbf{s} \mid \theta)\f$
     */
    double log_probability(const std::vector<state_id>& seq) const;

    /**
     * @return \f$P(\mathbf{s} \mid \theta)\f$
     */
    double probability(const std::vector<state_id>& seq) const;

    /**
     * @return \f$P(s_{t} \mid s_{f}, \theta)\f$
     */
    double transition_probability(state_id from, state_id to) const;

    /**
     * @return \f$P(s \mid \theta)\f$
     */
    double initial_probability(state_id s) const;

    /**
     * Saves a Markov model to a stream.
     */
    template <class OutputStream>
    void save(OutputStream& os) const
    {
        io::packed::write(os, initial_prob_);
        io::packed::write(os, trans_prob_);
        io::packed::write(os, prior_);
    }

  private:
    std::vector<double> initial_prob_;
    util::dense_matrix<double> trans_prob_;
    stats::dirichlet<state_id> prior_;
};
}
}
#endif
