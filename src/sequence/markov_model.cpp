/**
 * @file markov_model.cpp
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include "meta/sequence/markov_model.h"

namespace meta
{
namespace sequence
{

markov_model::expected_counts_type::expected_counts_type(
    uint64_t num_states, stats::dirichlet<state_id> prior)
    : initial_count_(num_states),
      trans_count_{num_states, num_states},
      prior_{std::move(prior)}
{
    // nothing
}

void markov_model::expected_counts_type::increment(
    const std::vector<state_id>& seq, double amount)
{
    increment_initial(seq[0], amount);
    for (uint64_t t = 1; t < seq.size(); ++t)
        increment_transition(seq[t - 1], seq[t], amount);
}

void markov_model::expected_counts_type::increment_initial(state_id s,
                                                           double amount)
{
    initial_count_[s] += amount;
}

void markov_model::expected_counts_type::increment_transition(state_id from,
                                                              state_id to,
                                                              double amount)
{
    trans_count_(from, to) += amount;
}

auto markov_model::expected_counts_type::
operator+=(const expected_counts_type& other) -> expected_counts_type&
{
    std::transform(initial_count_.begin(), initial_count_.end(),
                   other.initial_count_.begin(), initial_count_.begin(),
                   [](double mic, double oic) { return mic + oic; });

    for (state_id s_i{0}; s_i < trans_count_.rows(); ++s_i)
    {
        std::transform(trans_count_.begin(s_i), trans_count_.end(s_i),
                       other.trans_count_.begin(s_i), trans_count_.begin(s_i),
                       [](double mtc, double otc) { return mtc + otc; });
    }

    return *this;
}

markov_model::markov_model(uint64_t num_states,
                           stats::dirichlet<state_id> prior)
    : initial_prob_(num_states),
      trans_prob_{num_states, num_states},
      prior_{std::move(prior)}
{
    for (state_id s_i{0}; s_i < num_states; ++s_i)
    {
        initial_prob_[s_i] = (1.0 + prior_.pseudo_counts(s_i))
                             / (num_states + prior_.pseudo_counts());

        for (state_id s_j{0}; s_j < num_states; ++s_j)
        {
            trans_prob_(s_i, s_j) = (1.0 + prior_.pseudo_counts(s_j))
                                    / (num_states + prior.pseudo_counts());
        }
    }
}

markov_model::markov_model(expected_counts_type&& counts)
    : initial_prob_{std::move(counts.initial_count_)},
      trans_prob_{std::move(counts.trans_count_)},
      prior_{std::move(counts.prior_)}
{
    // normalize probability estimates
    auto inorm
        = std::accumulate(initial_prob_.begin(), initial_prob_.end(), 0.0);
    for (state_id s_i{0}; s_i < num_states(); ++s_i)
    {
        initial_prob_[s_i] = (initial_prob_[s_i] + prior_.pseudo_counts(s_i))
                             / (inorm + prior_.pseudo_counts());

        auto tnorm = std::accumulate(trans_prob_.begin(s_i),
                                     trans_prob_.end(s_i), 0.0);
        for (state_id s_j{0}; s_j < num_states(); ++s_j)
        {
            trans_prob_(s_i, s_j)
                = (trans_prob_(s_i, s_j) + prior_.pseudo_counts(s_i))
                  / (tnorm + prior_.pseudo_counts());
        }
    }
}

auto markov_model::expected_counts() const -> expected_counts_type
{
    return {num_states(), prior_};
}

const stats::dirichlet<state_id>& markov_model::prior() const
{
    return prior_;
}

uint64_t markov_model::num_states() const
{
    return initial_prob_.size();
}

double markov_model::log_probability(const std::vector<state_id>& seq) const
{
    assert(seq.size() > 0);
    double log_prob = std::log(initial_prob_[seq[0]]);
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        log_prob += std::log(trans_prob_(seq[t - 1], seq[t]));
    }
    return log_prob;
}

double markov_model::probability(const std::vector<state_id>& seq) const
{
    return std::exp(log_probability(seq));
}

double markov_model::transition_probability(state_id from, state_id to) const
{
    return trans_prob_(from, to);
}

double markov_model::initial_probability(state_id s) const
{
    return initial_prob_[s];
}
}
}
