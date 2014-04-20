/**
 * @file crf.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <random>
#include <numeric>
#include <unordered_set>
#include <map>
#include "sequence/crf.h"
#include "util/mapping.h"
#include "util/optional.h"
#include "util/progress.h"
#include "util/time.h"

namespace meta
{
namespace sequence
{

void crf::scorer::score(const crf& model, const sequence& seq)
{
    transition_scores(model);
    state_scores(model, seq);
}

void crf::scorer::transition_scores(const crf& model)
{
    auto num_labels = model.label_id_mapping_.size();
    trans_.resize(num_labels, num_labels);
    trans_exp_.resize(num_labels, num_labels);
    for (label_id outer{0}; outer < num_labels; ++outer)
    {
        for (const auto& idx : model.trans_range(outer))
            trans_(outer, model.transition(idx)) = model.trans_weight(idx)
                                                   * model.scale_;

        // exponentiate and store in trans_exp_
        std::transform(trans_.begin(outer), trans_.end(outer),
                       trans_exp_.begin(outer), [](double val)
        { return std::exp(val); });
    }
}

void crf::scorer::state_scores(const crf& model, const sequence& seq)
{
    auto num_labels = model.label_id_mapping_.size();
    state_.resize(seq.size(), num_labels);
    state_exp_.resize(seq.size(), num_labels);
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        for (const auto& pair : seq[t].features())
        {
            auto value = model.scale_ * pair.second;
            for (const auto& idx : model.obs_range(pair.first))
            {
                auto lbl = model.observation(idx);
                state_(t, lbl) += model.obs_weight(idx) * value;
            }
        }

        // exponentiate and store in state_exp_
        std::transform(state_.begin(t), state_.end(t),
                       state_exp_.begin(t), [](double val)
        { return std::exp(val); });
    }
}

double crf::scorer::state(uint64_t time, label_id lbl) const
{
    return state_(time, lbl);
}

double crf::scorer::state_exp(uint64_t time, label_id lbl) const
{
    return state_exp_(time, lbl);
}

double crf::scorer::trans(label_id from, label_id to) const
{
    return trans_(from, to);
}

double crf::scorer::trans_exp(label_id from, label_id to) const
{
    return trans_exp_(from, to);
}

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
    std::map<feature_id, std::unordered_set<label_id>> obs_feats;
    std::map<label_id, std::unordered_set<label_id>> trans_feats;

    {
        printing::progress progress{" > Feature generation: ", examples.size()};
        uint64_t idx = 0;
        for (const auto& seq : examples)
        {
            progress(++idx);
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
    }

    observation_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/observation_ranges.vector", obs_feats.size() + 1};
    transition_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/transition_ranges.vector", trans_feats.size() + 1};

    uint64_t obs_size = 0;
    uint64_t id = 0;
    for (const auto& pair : obs_feats)
    {
        if (id++ != pair.first)
            throw std::runtime_error{"wat"};
        (*observation_ranges_)[pair.first] = obs_size;
        obs_size += pair.second.size();
    }
    (*observation_ranges_)[observation_ranges_->size() - 1] = obs_size;

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
    id = 0;
    for (const auto& pair : trans_feats)
    {
        if (id++ != pair.first)
            throw std::runtime_error{"wat_trans"};
        (*transition_ranges_)[pair.first] = trans_size;
        trans_size += pair.second.size();
    }
    (*transition_ranges_)[transition_ranges_->size() - 1] = trans_size;

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

    LOG(info) << "Num features: " << transition_weights_->size()
                                     + observation_weights_->size() << ENDLG;
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
    return util::range((*observation_ranges_)[fid],
                       crf_feature_id{(*observation_ranges_)[fid + 1] - 1});
}

auto crf::trans_range(label_id lbl) const -> feature_range
{
    return util::range((*transition_ranges_)[lbl],
                       crf_feature_id{(*transition_ranges_)[lbl + 1] - 1});
}

label_id crf::observation(crf_feature_id fid) const
{
    return (*observations_)[fid];
}

label_id crf::transition(crf_feature_id fid) const
{
    return (*transitions_)[fid];
}

double crf::calibrate(parameters params, const std::vector<uint64_t>& indices,
                      const std::vector<sequence>& examples)
{
    auto num_samples = std::min(params.calibration_samples, indices.size());
    std::vector<uint64_t> samples{indices.begin(),
                                  indices.begin() + num_samples};

    double initial_loss = 0;
    printing::progress progress{" > Initial loss: ", num_samples};
    progress.print_endline(false);

    for (uint64_t idx = 0; idx < num_samples; ++idx)
    {
        progress(idx);

        const auto& seq = examples[samples[idx]];
        scorer_.score(*this, seq);
        auto fwd = forward(seq);
        auto bwd = backward(seq, fwd);
        initial_loss += loss(seq, fwd);
    }
    progress.end();
    progress.clear();
    LOG(progress) << "\r > Initial loss: " << initial_loss << '\n' << ENDLG;

    auto eta = params.calibration_eta;
    auto best_eta = eta;
    auto best_loss = initial_loss;
    uint64_t trial = 0;
    bool increase = true;
    while (trial < params.calibration_trials)
    {
        reset();
        // set learning rate to eta
        params.t0 = 1.0 / (params.lambda * eta);

        std::stringstream ss;
        ss << " > Trial " << trial + 1 << ": ";
        printing::progress progress{ss.str(), num_samples};
        progress.print_endline(false);
        auto loss = epoch(params, progress, 0, samples, examples);
        loss += 0.5 * l2norm() * params.lambda * examples.size();
        progress.end();
        progress.clear();

        if (std::isfinite(loss) && loss < initial_loss)
        {
            LOG(progress) << "\r" << ss.str() << "eta=" << eta
                          << ", loss=" << loss << " (possible)\n" << ENDLG;
            ++trial;

            if (loss < best_loss)
            {
                best_eta = eta;
                best_loss = loss;
            }

            if (increase)
                eta *= params.calibration_rate;
            else
                eta /= params.calibration_rate;
        }
        else
        {
            LOG(progress) << "\r" << ss.str() << "eta=" << eta
                          << ", loss=" << loss << " (worse)\n" << ENDLG;
            increase = false;
            eta = params.calibration_eta / params.calibration_rate;
        }
    }

    LOG(info) << "Picked learning rate: " << best_eta << ENDLG;
    reset();

    return 1.0 / (params.lambda * best_eta);
}

double crf::train(parameters params, const std::vector<sequence>& examples)
{
    initialize(examples);

    params.lambda = 2.0 * params.c2 / examples.size();

    std::vector<uint64_t> indices(examples.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng{std::random_device{}()};
    std::shuffle(indices.begin(), indices.end(), rng);

    params.t0 = calibrate(params, indices, examples);

    std::vector<double> old_loss(params.period);

    double loss = 0;
    double delta = 0;
    for (uint64_t iter = 1; iter <= params.max_iters; ++iter)
    {
        std::stringstream ss;
        ss << " > Epoch " << iter << ": ";
        printing::progress progress{ss.str(), examples.size()};
        progress.print_endline(false);
        std::shuffle(indices.begin(), indices.end(), rng);
        auto time = common::time<std::chrono::milliseconds>([&]()
        {
            loss = epoch(params, progress, iter - 1, indices, examples);
        });
        if (scale_ < 1e-9)
            rescale();
        auto l2 = l2norm();
        loss += 0.5 * l2 * params.lambda * examples.size();
        progress.end();
        progress.clear();
        ss << "elapsed time=" << time.count() / 1000.0 << "s";
        ss << ", l2norm=" << std::sqrt(l2) << ", loss=" << loss;
        if (iter > params.period)
        {
            delta = (old_loss[(iter - 1) % params.period] - loss) / loss;
            ss << ", improvement=" << delta;
            if (iter % params.period == 0)
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
        LOG(progress) << "\r" << ss.str() << "\n" << ENDLG;
        old_loss[(iter - 1) % params.period] = loss;
    }
    rescale();
    return loss;
}

double crf::epoch(parameters params, printing::progress& progress,
                  uint64_t iter, const std::vector<uint64_t>& indices,
                  const std::vector<sequence>& examples)
{
    double sum_loss = 0;
    for (uint64_t i = 0; i < indices.size(); ++i)
    {
        progress(i);
        const auto& elem = examples[indices[i]];
        sum_loss += iteration(params, iter * indices.size() + i, elem);
    }
    return sum_loss;
}

double crf::iteration(parameters params, uint64_t iter,
                      const sequence& seq)
{
    double lr = 1 / (params.lambda * (params.t0 + iter));
    scale_ *= (1 - params.lambda * lr);
    auto gain = lr / scale_;

    scorer_.score(*this, seq);
    auto fwd = forward(seq);
    auto bwd = backward(seq, fwd);

    auto state_mrg = state_marginals(fwd, bwd);
    auto trans_mrg = transition_marginals(fwd, bwd);

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
            for (const auto& idx : obs_range(pair.first))
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
            for (const auto& idx : trans_range(*prev))
            {
                if (transition(idx) == lbl)
                {
                    trans_weight(idx) += gain;
                    break;
                }
            }
        }

        prev = lbl;
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
            for (const auto& idx : obs_range(pair.first))
            {
                auto lbl = observation(idx);
                obs_weight(idx) += gain * pair.second * state_mrg(t, lbl);
            }
        }
    }

    for (label_id i{0}; i < label_id_mapping_.size(); ++i)
    {
        for (const auto& idx : trans_range(i))
        {
            auto j = transition(idx);
            trans_weight(idx) += gain * trans_mrg(i, j);
        }
    }
}



forward_trellis crf::forward(const sequence& seq) const
{
    forward_trellis table{seq.size(), label_id_mapping_.size()};

    // initialize first column of trellis
    for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        table.probability(0, lbl, scorer_.state_exp(0, lbl));
    // normalize to avoid underflow
    table.normalize(0);

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            auto score = scorer_.state_exp(t ,lbl);
            double sum = 0;
            for (label_id in{0}; in < label_id_mapping_.size(); ++in)
            {
                sum += table.probability(t - 1, in)
                       * scorer_.trans_exp(in, lbl);
            }
            table.probability(t, lbl, score * sum);
        }
        // normalize to avoid underflow
        table.normalize(t);
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
        for (label_id i{0}; i < label_id_mapping_.size(); ++i)
        {
            double sum = 0;
            for (label_id j{0}; j < label_id_mapping_.size(); ++j)
            {
                sum += table.probability(t, j) * scorer_.state_exp(t, j)
                       * scorer_.trans_exp(i, j);
            }
            table.probability(t - 1, i, fwd.normalizer(t - 1) * sum);
        }
    }

    return table;
}

auto crf::state_marginals(const forward_trellis& fwd,
                          const trellis& bwd) const -> double_matrix
{
    double_matrix table{fwd.size(), label_id_mapping_.size()};

    for (uint64_t t = 0; t < table.rows(); ++t)
    {
        for (label_id lbl{0}; lbl < table.columns(); ++lbl)
        {
            table(t, lbl) = fwd.probability(t, lbl) * bwd.probability(t, lbl)
                            * (1.0 / fwd.normalizer(t));
        }
    }
    return table;
}

auto crf::transition_marginals(const forward_trellis& fwd,
                               const trellis& bwd) const -> double_matrix
{
    double_matrix table{label_id_mapping_.size(), label_id_mapping_.size()};

    for (uint64_t t = 0; t < fwd.size() - 1; ++t)
    {
        for (label_id lbl{0}; lbl < table.rows(); ++lbl)
        {
            for (label_id in{0}; in < table.columns(); ++in)
            {
                table(lbl, in) += fwd.probability(t, lbl)
                                  * scorer_.trans_exp(lbl, in)
                                  * scorer_.state_exp(t + 1, in)
                                  * bwd.probability(t + 1, in);
            }
        }
    }
    return table;
}

double crf::loss(const sequence& seq, const forward_trellis& fwd)
{
    util::optional<label_id> prev;
    double score = 0;
    double normalizer = 0;
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        auto curr = label(seq[t].tag());
        score += scorer_.state(t, curr);
        if (prev)
            score += scorer_.trans(*prev, curr);

        // factor in log normalizer: log(Z(x)) = - \sum_t log(scale[t])
        // and loss = -score(x) + log(Z(x)), so we add on log(scale[t])
        normalizer += std::log(fwd.normalizer(t));
        prev = curr;
    }
    return -score - normalizer;
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
