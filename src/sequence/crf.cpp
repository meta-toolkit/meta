/**
 * @file crf.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <random>
#include <numeric>
#include <unordered_set>
#include <map>
#include <set>
#include "sequence/crf.h"
#include "util/mapping.h"
#include "util/optional.h"
#include "util/progress.h"
#include "util/time.h"

namespace meta
{
namespace sequence
{

crf::crf(const std::string& prefix)
    : scale_{1}, prefix_{prefix}
{
    filesystem::make_directory(prefix_);
}

label_id crf::label(tag_t tag)
{
    if (!label_id_mapping_.contains_key(tag))
    {
        label_id id(label_id_mapping_.size());
        label_id_mapping_.insert(tag, id);
    }
    return label_id_mapping_.get_value(tag);
}

void crf::initialize(const std::vector<sequence>& examples)
{

    std::map<feature_id, std::set<label_id>> obs_feats;
    std::map<label_id, std::set<label_id>> trans_feats;

    for (const auto& seq : examples)
    {
        util::optional<label_id> prev;
        for (uint64_t t = 0; t < seq.size(); ++t)
        {
            // build label id mapping by side-effect
            auto lbl = label(seq[t].tag());

            // observation features
            for (const auto& pair : seq[t].features())
                obs_feats[pair.first].insert(lbl);

            // transition features
            if (prev)
                trans_feats[*prev].insert(lbl);

            prev = lbl;
        }
    }

    observation_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/observation_ranges.vector", obs_feats.size()};
    transition_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/transition_ranges.vector", trans_feats.size()};

    uint64_t obs_size = 0;
    for (const auto& pair : obs_feats)
    {
        (*observation_ranges_)[pair.first] = obs_size;
        obs_size += pair.second.size();
    }

    observation_weights_ = util::disk_vector<double>{
        prefix_ + "/observation_weights.vector", obs_size};
    observations_ = util::disk_vector<label_id>{
        prefix_ + "/observations.vector", obs_size};

    uint64_t idx = 0;
    for (const auto& pair : obs_feats)
    {
        for (const auto& lbl : pair.second)
        {
            (*observations_)[idx] = lbl;
            (*observation_weights_)[idx] = 0;
            ++idx;
        }
    }

    uint64_t trans_size = 0;
    for (const auto& pair : trans_feats)
    {
        (*transition_ranges_)[pair.first] = trans_size;
        trans_size += pair.second.size();
    }

    transition_weights_ = util::disk_vector<double>{
        prefix_ + "/transition_weights.vector", trans_size};
    transitions_ = util::disk_vector<label_id>{
        prefix_ + "/transitions.vector", trans_size};

    idx = 0;
    for (const auto& pair : trans_feats)
    {
        for (const auto& lbl : pair.second)
        {
            (*transitions_)[idx] = lbl;
            (*transition_weights_)[idx] = 0;
            ++idx;
        }
    }
}

void crf::reset()
{
    for (auto& w : *observation_weights_)
        w = 0;
    for (auto& w : *transition_weights_)
        w = 0;
    scale_ = 1;
}

const double& crf::obs_weight(crf_feature_id idx) const
{
    return (*observation_weights_)[idx];
}

double& crf::obs_weight(crf_feature_id idx)
{
    return (*observation_weights_)[idx];
}

const double& crf::trans_weight(crf_feature_id idx) const
{
    return (*transition_weights_)[idx];
}

double& crf::trans_weight(crf_feature_id idx)
{
    return (*transition_weights_)[idx];
}

auto crf::obs_range(feature_id fid) const -> feature_range
{
    if (fid + 1 > observation_ranges_->size())
        return {(*observation_ranges_)[fid], crf_feature_id{observation_weights_->size()}};
    return {(*observation_ranges_)[fid], (*observation_ranges_)[fid + 1]};
}

auto crf::trans_range(label_id lbl) const -> feature_range
{
    if (lbl + 1 > transition_ranges_->size())
        return {(*transition_ranges_)[lbl], crf_feature_id{transition_weights_->size()}};
    return {(*transition_ranges_)[lbl], (*transition_ranges_)[lbl + 1]};
}

label_id crf::observation(crf_feature_id fid) const
{
    return (*observations_)[fid];
}

label_id crf::transition(crf_feature_id fid) const
{
    return (*transitions_)[fid];
}

double crf::train(parameters params, const std::vector<sequence>& examples)
{
    initialize(examples);
    std::vector<uint64_t> indices(examples.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng{std::random_device{}()};

    double old_loss = std::numeric_limits<double>::max();
    double loss = old_loss;
    double delta;
    for (uint64_t iter = 1; iter <= params.max_iters; ++iter)
    {
        std::stringstream ss;
        ss << " > Epoch " << iter << ": ";
        printing::progress progress{ss.str(), examples.size()};
        progress.print_endline(false);
        std::shuffle(indices.begin(), indices.end(), rng);
        auto time = common::time<std::chrono::milliseconds>([&]()
        {
            loss = epoch(params, progress, indices, examples);
        });
        progress.end();
        progress.clear();
        ss << "elapsed time=" << time.count() / 1000.0 << "s";
        ss << ", loss=" << loss;
        if (iter > 1)
        {
            delta = std::abs(old_loss - loss) / loss;
            ss << ", improvement=" << delta;
            if (iter % params.period == 0)
            {
                if (iter > params.period)
                {
                    if (delta < params.delta)
                    {
                        ss << ", converged!";
                        LOG(progress) << "\r" << ss.str() << "\n" << ENDLG;
                        rescale();
                        return loss;
                    }
                }
            }
        }
        LOG(progress) << "\r" << ss.str() << "\n" << ENDLG;
        old_loss = loss;
        loss = 0;
    }
    rescale();
    return old_loss;
}

double crf::epoch(parameters params, printing::progress& progress,
                  const std::vector<uint64_t>& indices,
                  const std::vector<sequence>& examples)
{
    double sum_loss = 0;
    for (uint64_t i = 0; i < indices.size(); ++i)
    {
        progress(i);
        const auto& elem = examples[indices[i]];
        sum_loss += iteration(params, elem);

        if (scale_ < 1e-9)
            rescale();
    }
    return sum_loss + l2norm() * 0.5 * params.lambda;
}

double crf::iteration(parameters params, const sequence& seq)
{
    state_scores(seq);
    transition_scores();
    auto fwd = forward(seq);
    auto bwd = backward(seq, fwd);

    auto state_mrg = state_marginals(fwd, bwd);
    auto trans_mrg = transition_marginals(fwd, bwd);

    scale_ *= (1 - params.lambda * params.lr);
    auto gain = params.lr / scale_;
    gradient_observation_expectation(seq, gain);
    gradient_model_expectation(seq, -1.0 * gain, state_mrg, trans_mrg);

    return loss(seq, fwd);
}

void crf::gradient_observation_expectation(const sequence& seq, double gain)
{
    util::optional<label_id> prev;
    for (const auto& obs : seq)
    {
        auto lbl = label(obs.tag());
        for (const auto& pair : obs.features())
        {
            const auto& range = obs_range(pair.first);
            for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
            {
                if (observation(idx) == lbl)
                {
                    obs_weight(idx) += gain * pair.second;
                    break;
                }
            }
        }

        if (prev)
        {
            const auto& range = trans_range(*prev);
            for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
            {
                if (transition(idx) == lbl)
                {
                    trans_weight(idx) += gain;
                    break;
                }
            }
        }
    }
}

void crf::gradient_model_expectation(const sequence& seq, double gain,
                                     const double_matrix& state_mrg,
                                     const double_matrix& trans_mrg)
{
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        for (const auto& pair : seq[t].features())
        {
            const auto& range = obs_range(pair.first);
            for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
            {
                auto lbl = observation(idx);
                obs_weight(idx) += gain * pair.second * state_mrg[t][lbl];
            }
        }
    }

    for (label_id i{0}; i < label_id_mapping_.size(); ++i)
    {
        const auto& range = trans_range(i);
        for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
        {
            auto j = transition(idx);
            trans_weight(idx) += gain * trans_mrg[i][j];
        }
    }
}

void crf::state_scores(const sequence& seq)
{
    state_.resize(seq.size());
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        state_[t].resize(label_id_mapping_.size());
        std::fill(state_[t].begin(), state_[t].end(), 0);
        for (const auto& pair : seq[t].features())
        {
            const auto& range = obs_range(pair.first);
            for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
            {
                auto lbl = observation(idx);
                state_[t][lbl] += obs_weight(idx) * scale_ * pair.second;
            }
        }

        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            state_[t][lbl] = std::exp(state_[t][lbl]);
        }
    }
}

void crf::transition_scores()
{
    trans_.resize(label_id_mapping_.size());
    auto num_labels = label_id_mapping_.size();
    for (label_id outer{0}; outer < num_labels; ++outer)
    {
        trans_[outer].resize(num_labels);
        std::fill(trans_[outer].begin(), trans_[outer].end(), 1);
        const auto& range = trans_range(outer);
        for (crf_feature_id idx{range.start}; idx < range.end; ++idx)
            trans_[outer][transition(idx)] =
                std::exp(trans_weight(idx) * scale_);
    }
}

forward_trellis crf::forward(const sequence& seq) const
{
    forward_trellis table{seq.size(), label_id_mapping_.size()};

    // initialize first column of trellis
    double normalizer = 0;
    for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
    {
        auto val = state_[0][lbl];
        table.probability(0, lbl, val);
        normalizer += val;
    }
    // normalize to avoid underflow
    table.normalize(0, normalizer > 0 ? 1 / normalizer : 1);

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        double normalizer = 0;
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            auto score = state_[t][lbl];
            double sum = 0;
            for (label_id in{0}; in < label_id_mapping_.size(); ++in)
            {
                sum += table.probability(t - 1, in) * trans_[in][lbl];
            }
            score *= sum;
            table.probability(t, lbl, score);
            normalizer += sum;
        }
        // normalize to avoid underflow
        table.normalize(t, normalizer > 0 ? 1.0 / normalizer : 1);
    }

    return table;
}

trellis crf::backward(const sequence& seq, const forward_trellis& fwd) const
{
    trellis table{seq.size(), label_id_mapping_.size()};

    // initialize last column of the trellis
    for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
    {
        auto val = fwd.normalizer(seq.size() - 1);
        table.probability(seq.size() - 1, lbl, val);
    }

    // to avoid unsigned weirdness, t is really t+1, so we are actually
    // going to compute for index t-1 here to compute beta[t]
    for (uint64_t t = seq.size() - 1; t > 0; --t)
    {
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            double sum = 0;
            for (label_id in{0}; in < label_id_mapping_.size(); ++in)
            {
                sum +=
                    state_[t][in] * trans_[lbl][in] * table.probability(t, in);
            }
            table.probability(t - 1, lbl, sum * fwd.normalizer(t - 1));
        }
    }

    return table;
}

auto crf::state_marginals(const forward_trellis& fwd,
                          const trellis& bwd) const -> double_matrix
{
    double_matrix table(fwd.size());

    for (uint64_t t = 0; t < table.size(); ++t)
    {
        table[t].resize(label_id_mapping_.size());
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            table[t][lbl] = fwd.probability(t, lbl) * bwd.probability(t, lbl)
                            * (1.0 / fwd.normalizer(t));
        }
    }
    return table;
}

auto crf::transition_marginals(const forward_trellis& fwd,
                               const trellis& bwd) const -> double_matrix
{
    double_matrix table(label_id_mapping_.size());
    for (label_id lbl{0}; lbl < table.size(); ++lbl)
        table[lbl].resize(table.size());

    for (uint64_t t = 0; t < fwd.size() - 1; ++t)
    {
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            for (label_id in{0}; lbl < label_id_mapping_.size(); ++lbl)
            {
                table[lbl][lbl] += fwd.probability(t, lbl)
                                   * state_[t + 1][in]
                                   * trans_[lbl][in]
                                   * bwd.probability(t + 1, in);
            }
        }
    }
    return table;
}

double crf::loss(const sequence& seq, const forward_trellis& fwd)
{
    util::optional<label_id> prev;
    double result = 0;
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        auto curr = label(seq[t].tag());
        result -= std::log(state_[t][curr]);

        if (prev)
            result -= std::log(trans_[*prev][curr]);

        // factor in log normalizer: log(Z(x)) = - \sum_t log(scale[t])
        // and loss = -score(x) + log(Z(x)), so we add on log(scale[t])
        result -= std::log(fwd.normalizer(t));
        prev = curr;
    }
    return result;
}

double crf::l2norm() const
{
    double norm = 0;
    for (const auto& w : *observation_weights_)
        norm += w * w;
    for (const auto& w : *transition_weights_)
        norm += w * w;
    return norm * scale_ * scale_;
}

void crf::rescale()
{
    for (auto& w : *observation_weights_)
        w *= scale_;
    for (auto& w : *transition_weights_)
        w *= scale_;
    scale_ = 1;
}
}
}
