/**
 * @file sequence_observations.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/sequence/hmm/sequence_observations.h"

namespace meta
{
namespace sequence
{
namespace hmm
{

sequence_observations::expected_counts_type::expected_counts_type(
    uint64_t num_hmm_states, uint64_t num_markov_states,
    stats::dirichlet<state_id> prior)
{
    counts_.reserve(num_hmm_states);
    for (state_id s_i{0}; s_i < num_hmm_states; ++s_i)
        counts_.emplace_back(num_markov_states, prior);
}

void sequence_observations::expected_counts_type::increment(
    const observation_type& seq, state_id s_i, double amount)
{
    counts_[s_i].increment(seq, amount);
}

auto sequence_observations::expected_counts_type::
operator+=(const expected_counts_type& other) -> expected_counts_type&
{
    for (state_id s_i{0}; s_i < counts_.size(); ++s_i)
    {
        counts_[s_i] += other.counts_[s_i];
    }
    return *this;
}

sequence_observations::sequence_observations(uint64_t num_hmm_states,
                                             uint64_t num_markov_states,
                                             stats::dirichlet<state_id> prior)
{
    models_.reserve(num_hmm_states);
    for (uint64_t h = 0; h < num_hmm_states; ++h)
        models_.emplace_back(num_markov_states, prior);
}

sequence_observations::sequence_observations(expected_counts_type&& counts)
    : models_{[&]() {
          std::vector<markov_model> models;
          models.reserve(counts.counts_.size());
          for (auto& ec : counts.counts_)
              models.emplace_back(std::move(ec));
          return models;
      }()}
{
    // nothing
}

auto sequence_observations::expected_counts() const -> expected_counts_type
{
    return {num_states(), models_.front().num_states(),
            models_.front().prior()};
}

uint64_t sequence_observations::num_states() const
{
    return models_.size();
}

double sequence_observations::probability(const observation_type& obs,
                                          state_id s_i) const
{
    return models_[s_i].probability(obs);
}

double sequence_observations::log_probability(const observation_type& obs,
                                              state_id s_i) const
{
    return models_[s_i].log_probability(obs);
}

const markov_model& sequence_observations::distribution(state_id s_i) const
{
    return models_[s_i];
}
}
}
}
