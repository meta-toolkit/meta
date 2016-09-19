/**
 * @file word_observations.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SEQUENCE_HMM_WORD_OBS_H_
#define META_SEQUENCE_HMM_WORD_OBS_H_

#include "meta/meta.h"
#include "meta/sequence/hmm/hmm.h"
#include "meta/stats/multinomial.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

/**
 * A multinomial observation distribution for HMMs.
 */
template <class ObservationType = term_id>
class discrete_observations
{
  public:
    using observation_type = ObservationType;
    using conditional_distribution_type = stats::multinomial<observation_type>;

    /**
     * E-step scratch space for computing expected counts.
     */
    class expected_counts_type
    {
      public:
        friend discrete_observations;

        expected_counts_type(uint64_t num_states,
                        stats::dirichlet<observation_type> prior)
            : obs_dist_(num_states, prior)
        {
            // nothing
        }

        void increment(const observation_type& obs, state_id s_i, double count)
        {
            obs_dist_[s_i].increment(obs, count);
        }

        expected_counts_type& operator+=(const expected_counts_type& other)
        {
            for (state_id s_i{0}; s_i < obs_dist_.size(); ++s_i)
                obs_dist_[s_i] += other.obs_dist_[s_i];
            return *this;
        }

      private:
        std::vector<conditional_distribution_type> obs_dist_;
    };

    /**
     * Initializes each multinomial distribution for each hidden state
     * randomly by using the provided random number generator.
     */
    template <class Generator>
    discrete_observations(uint64_t num_states, uint64_t num_observations,
                          Generator&& rng,
                          stats::dirichlet<observation_type>&& prior)
        : obs_dist_(num_states, prior)
    {
        for (auto& dist : obs_dist_)
        {
            for (observation_type o{0}; o < num_observations; ++o)
            {
                auto rnd = random::bounded_rand(rng, 65536);
                auto val = (rnd / 65536.0) / num_observations;

                dist.increment(o, val);
            }
        }
    }

    /**
     * Re-estimates the multinomials given expected_counts.
     */
    discrete_observations(expected_counts_type&& counts)
      : obs_dist_(std::move(counts.obs_dist_))
    {
       // nothing
    }

    /**
     * Obtains an expected_counts_type suitable for re-estimating this
     * distribution.
     */
    expected_counts_type expected_counts() const
    {
        return {num_states(), obs_dist_.front().prior()};
    }

    uint64_t num_states() const
    {
        return obs_dist_.size();
    }

    double probability(observation_type obs, state_id s_i) const
    {
        return obs_dist_[s_i].probability(obs);
    }

    const conditional_distribution_type& distribution(state_id s_i) const
    {
        return obs_dist_[s_i];
    }

  private:
    std::vector<conditional_distribution_type> obs_dist_;
};
}
}
}
#endif
