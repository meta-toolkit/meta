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

    uint64_t num_states() const
    {
        return obs_dist_.size();
    }

    discrete_observations blank() const
    {
        return {obs_dist_.size(), prior()};
    }

    const stats::dirichlet<observation_type>& prior() const
    {
        return obs_dist_.front().prior();
    }

    double probability(observation_type obs, state_id s_i) const
    {
        return obs_dist_[s_i].probability(obs);
    }

    void increment(observation_type obs, state_id s_i, double amount)
    {
        obs_dist_[s_i].increment(obs, amount);
    }

  private:
    std::vector<stats::multinomial<observation_type>> obs_dist_;
};
}
}
}
#endif
