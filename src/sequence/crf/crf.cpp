/**
 * @file crf.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <random>
#include <numeric>
#include <map>
#include <set>
#include "meta/logging/logger.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/scorer.h"
#include "meta/util/mapping.h"
#include "meta/util/optional.h"
#include "meta/util/progress.h"
#include "meta/util/time.h"

namespace meta
{
namespace sequence
{

crf::crf(const std::string& prefix) : scale_{1}, prefix_{prefix}
{
    if (filesystem::file_exists(prefix_ + "/observation_ranges.vector"))
        load_model();
    else
        filesystem::make_directory(prefix_);
}

void crf::load_model()
{
    // ranges
    observation_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/observation_ranges.vector"};
    transition_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/transition_ranges.vector"};

    // observations
    observation_weights_
        = util::disk_vector<double>{prefix_ + "/observation_weights.vector"};
    observations_
        = util::disk_vector<label_id>{prefix_ + "/observations.vector"};

    // transitions
    transition_weights_
        = util::disk_vector<double>{prefix_ + "/transition_weights.vector"};
    transitions_ = util::disk_vector<label_id>{prefix_ + "/transitions.vector"};

    num_labels_ = transition_ranges_->size() - 1;
}

void crf::initialize(const std::vector<sequence>& examples)
{
    std::map<feature_id, std::set<label_id>> obs_feats;
    std::map<label_id, std::set<label_id>> trans_feats;

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
                auto lbl = seq[t].label();

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

    num_labels_ = trans_feats.size();

    observation_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/observation_ranges.vector", obs_feats.size() + 1};
    transition_ranges_ = util::disk_vector<crf_feature_id>{
        prefix_ + "/transition_ranges.vector", trans_feats.size() + 1};

    crf_feature_id obs_size{0};
    for (const auto& pair : obs_feats)
    {
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

    crf_feature_id trans_size{0};
    for (const auto& pair : trans_feats)
    {
        (*transition_ranges_)[pair.first] = trans_size;
        trans_size += pair.second.size();
    }
    (*transition_ranges_)[transition_ranges_->size() - 1] = trans_size;

    transition_weights_ = util::disk_vector<double>{
        prefix_ + "/transition_weights.vector", trans_size};
    transitions_ = util::disk_vector<label_id>{prefix_ + "/transitions.vector",
                                               trans_size};

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

    LOG(info) << "Num features: "
              << transition_weights_->size() + observation_weights_->size()
              << ENDLG;
}

void crf::reset()
{
    for (auto& w : *observation_weights_)
        w = 0;
    for (auto& w : *transition_weights_)
        w = 0;
    scale_ = 1;
}

uint64_t crf::num_labels() const
{
    return num_labels_;
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
                       crf_feature_id((*observation_ranges_)[fid + 1] - 1));
}

auto crf::trans_range(label_id lbl) const -> feature_range
{
    return util::range((*transition_ranges_)[lbl],
                       crf_feature_id((*transition_ranges_)[lbl + 1] - 1));
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
    auto num_samples
        = std::min<uint64_t>(params.calibration_samples, indices.size());

    using diff_type = decltype(indices.begin())::difference_type;
    std::vector<uint64_t> samples{
        indices.begin(), indices.begin() + static_cast<diff_type>(num_samples)};

    double initial_loss = 0;
    printing::progress progress{" > Initial loss: ", num_samples};
    progress.print_endline(false);

    scorer scorer;
    for (uint64_t idx = 0; idx < num_samples; ++idx)
    {
        progress(idx);

        const auto& seq = examples[samples[idx]];
        scorer.score(*this, seq);
        scorer.forward();
        initial_loss += scorer.loss(seq);
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
        auto loss = epoch(params, progress, 0, samples, examples, scorer);
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

    scorer scorer;
    double loss = 0;
    double delta = 0;
    for (uint64_t iter = 1; iter <= params.max_iters; ++iter)
    {
        std::stringstream ss;
        ss << " > Epoch " << iter << ": ";
        printing::progress progress{ss.str(), examples.size()};
        progress.print_endline(false);
        std::shuffle(indices.begin(), indices.end(), rng);
        auto time = common::time<std::chrono::milliseconds>(
            [&]()
            {
                loss = epoch(params, progress, iter - 1, indices, examples,
                             scorer);
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
                  const std::vector<sequence>& examples, scorer& scorer)
{
    double sum_loss = 0;
    for (uint64_t i = 0; i < indices.size(); ++i)
    {
        progress(i);
        const auto& elem = examples[indices[i]];
        sum_loss += iteration(params, iter * indices.size() + i, elem, scorer);
    }
    return sum_loss;
}

double crf::iteration(parameters params, uint64_t iter, const sequence& seq,
                      scorer& scorer)
{
    double lr = 1 / (params.lambda * (params.t0 + iter));
    scale_ *= (1 - params.lambda * lr);
    auto gain = lr / scale_;

    scorer.score(*this, seq);
    scorer.marginals();

    gradient_observation_expectation(seq, gain);
    gradient_model_expectation(seq, -1.0 * gain, scorer);

    return scorer.loss(seq);
}

void crf::gradient_observation_expectation(const sequence& seq, double gain)
{
    util::optional<label_id> prev;
    for (const auto& obs : seq)
    {
        auto lbl = obs.label();
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
                                     const scorer& scr)
{
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        for (const auto& pair : seq[t].features())
        {
            for (const auto& idx : obs_range(pair.first))
            {
                auto lbl = observation(idx);
                obs_weight(idx)
                    += gain * pair.second * scr.state_marginal(t, lbl);
            }
        }
    }

    for (label_id i{0}; i < num_labels(); ++i)
    {
        for (const auto& idx : trans_range(i))
        {
            auto j = transition(idx);
            trans_weight(idx) += gain * scr.trans_marginal(i, j);
        }
    }
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
