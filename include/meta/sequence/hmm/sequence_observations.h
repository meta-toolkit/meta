/**
 * @file sequence_observations.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_HMM_SEQUENCE_OBS_H_
#define META_SEQUENCE_HMM_SEQUENCE_OBS_H_

#include "meta/sequence/hmm/hmm.h"
#include "meta/sequence/markov_model.h"
#include "meta/stats/multinomial.h"
#include "meta/util/traits.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

/**
 * A Markov Model observation distribution for HMMs. Each observation is
 * assumed to be a sequence of states. Each *HMM* state is modeled via a
 * separate Markov model.
 */
class sequence_observations
{
  public:
    using observation_type = std::vector<state_id>;
    using conditional_distribution_type = markov_model;

    /**
     * E-step scratch space for computing expected counts.
     */
    class expected_counts_type
    {
      public:
        friend sequence_observations;

        expected_counts_type(uint64_t num_hmm_states,
                             uint64_t num_markov_states,
                             stats::dirichlet<state_id> prior);

        void increment(const observation_type& seq, state_id s_i,
                       double amount);

        expected_counts_type& operator+=(const expected_counts_type& other);

      private:
        std::vector<markov_model::expected_counts_type> counts_;
    };

    /**
     * Initializes each state's Markov model randomly using the provided
     * random number generator.
     */
    template <class Generator>
    sequence_observations(uint64_t num_hmm_states, uint64_t num_markov_states,
                          Generator&& gen, stats::dirichlet<state_id> prior)
    {
        models_.reserve(num_hmm_states);
        for (uint64_t h = 0; h < num_hmm_states; ++h)
            models_.emplace_back(num_markov_states,
                                 std::forward<Generator>(gen), prior);
    }

    /**
     * Default initializes each state's Markov model. This is only useful
     * when setting values manually by using increment().
     */
    sequence_observations(uint64_t num_hmm_states, uint64_t num_markov_states,
                          stats::dirichlet<state_id> prior);

    /**
     * Re-estimates the Markov models given expected counts.
     */
    sequence_observations(expected_counts_type&& counts);

    /**
     * Loads a sequence observation distribution from an input stream.
     */
    template <class InputStream,
              class = util::disable_if_same_or_derived_t<sequence_observations,
                                                         InputStream>>
    sequence_observations(InputStream& is)
    {
        uint64_t size;
        if (io::packed::read(is, size) == 0)
            throw hmm_exception{
                "failed to load sequence_observations from stream"};

        models_.reserve(size);
        for (uint64_t i = 0; i < size; ++i)
            models_.emplace_back(is);
    }

    /**
     * Obtains an expected_counts_type suitable for re-estimating this
     * distribution.
     */
    expected_counts_type expected_counts() const;

    uint64_t num_states() const;

    double probability(const observation_type& obs, state_id s_i) const;

    double log_probability(const observation_type& obs, state_id s_i) const;

    const markov_model& distribution(state_id s_i) const;

    /**
     * Saves a sequence observation distribution to a stream.
     */
    template <class OutputStream>
    void save(OutputStream& os) const
    {
        io::packed::write(os, models_.size());
        for (const auto& model : models_)
            model.save(os);
    }

  private:
    std::vector<markov_model> models_;
};

template <>
struct hmm_traits<sequence_observations>
{
    using observation_type = sequence_observations::observation_type;
    using sequence_type = std::vector<observation_type>;
    using training_data_type = std::vector<sequence_type>;
    using forward_backward_type = logarithm_forward_backward;
};
}
}
}
#endif
