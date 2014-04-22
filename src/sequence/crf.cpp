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
#include "util/functional.h"

namespace meta
{
namespace sequence
{

void crf::scorer::score(const crf& model, const sequence& seq)
{
    transition_scores(model);
    state_scores(model, seq);
    fwd_ = util::nullopt;
    bwd_ = util::nullopt;
    state_mrg_ = util::nullopt;
    trans_mrg_ = util::nullopt;
}

void crf::scorer::transition_scores(const crf& model)
{
    auto num_labels = model.num_labels();
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
    auto num_labels = model.num_labels();
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

void crf::scorer::forward()
{
    fwd_ = forward_trellis{state_exp_.rows(), state_exp_.columns()};

    // initialize first column of trellis
    for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
        fwd_->probability(0, lbl, state_exp(0, lbl));
    // normalize to avoid underflow
    fwd_->normalize(0);

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < state_exp_.rows(); ++t)
    {
        for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
        {
            auto score = state_exp(t ,lbl);
            double sum = 0;
            for (label_id in{0}; in < state_exp_.columns(); ++in)
            {
                sum += fwd_->probability(t - 1, in) * trans_exp(in, lbl);
            }
            fwd_->probability(t, lbl, score * sum);
        }
        // normalize to avoid underflow
        fwd_->normalize(t);
    }
}

void crf::scorer::backward()
{
    if (!fwd_)
        forward();

    bwd_ = trellis{state_exp_.rows(), state_exp_.columns()};

    // initialize last column of the trellis
    for (label_id lbl{0}; lbl < state_exp_.columns(); ++lbl)
    {
        auto val = fwd_->normalizer(state_exp_.rows() - 1);
        bwd_->probability(state_exp_.rows() - 1, lbl, val);
    }

    // to avoid unsigned weirdness, t is really t+1, so we are actually
    // going to compute for index t-1 here to compute beta[t]
    for (uint64_t t = state_exp_.rows() - 1; t > 0; --t)
    {
        for (label_id i{0}; i < state_exp_.columns(); ++i)
        {
            double sum = 0;
            for (label_id j{0}; j < state_exp_.columns(); ++j)
            {
                sum += bwd_->probability(t, j) * state_exp(t, j)
                       * trans_exp(i, j);
            }
            bwd_->probability(t - 1, i, fwd_->normalizer(t - 1) * sum);
        }
    }
}

void crf::scorer::marginals()
{
    if (!fwd_)
        forward();

    if (!bwd_)
        backward();

    transition_marginals();
    state_marginals();
}

void crf::scorer::transition_marginals()
{
    trans_mrg_ = double_matrix{trans_exp_.rows(), trans_exp_.rows()};

    for (uint64_t t = 0; t < state_exp_.rows() - 1; ++t)
    {
        for (label_id lbl{0}; lbl < trans_mrg_->rows(); ++lbl)
        {
            for (label_id in{0}; in < trans_mrg_->columns(); ++in)
            {
                (*trans_mrg_)(lbl, in)
                    += fwd_->probability(t, lbl) * trans_exp(lbl, in)
                       * state_exp(t + 1, in) * bwd_->probability(t + 1, in);
            }
        }
    }
}

void crf::scorer::state_marginals()
{
    state_mrg_ = double_matrix{state_exp_.rows(), state_exp_.columns()};

    for (uint64_t t = 0; t < state_mrg_->rows(); ++t)
    {
        for (label_id lbl{0}; lbl < state_mrg_->columns(); ++lbl)
        {
            (*state_mrg_)(t, lbl) = fwd_->probability(t, lbl)
                                    * bwd_->probability(t, lbl)
                                    * (1.0 / fwd_->normalizer(t));
        }
    }
}

double crf::scorer::loss(const sequence& seq) const
{
    util::optional<label_id> prev;
    double score = 0;
    double normalizer = 0;
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        auto curr = seq[t].label();
        score += state(t, curr);
        if (prev)
            score += trans(*prev, curr);

        // factor in log normalizer: log(Z(x)) = - \sum_t log(scale[t])
        // and loss = -score(x) + log(Z(x)), so we add on log(scale[t])
        normalizer += std::log(fwd_->normalizer(t));
        prev = curr;
    }
    return -score - normalizer;
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

double crf::scorer::trans_marginal(label_id from, label_id to) const
{
    return (*trans_mrg_)(from, to);
}

double crf::scorer::state_marginal(uint64_t time, label_id lbl) const
{
    return (*state_mrg_)(time, lbl);
}

crf::viterbi_scorer::viterbi_scorer(const crf& model)
    : model_{&model}
{
    // these only ever need computing once because the underlying model is
    // not changing
    scorer_.transition_scores(*model_);
}

viterbi_trellis crf::viterbi_scorer::viterbi(const sequence& seq)
{
    // we only need the scores for the states as the transition scores, set
    // up during construction, will never change between sequences
    scorer_.state_scores(*model_, seq);

    viterbi_trellis table{seq.size(), model_->num_labels()};

    // initialize first column of trellis. We use the original state() and
    // trans() matrices because we are working in the log domain.
    for (label_id lbl{0}; lbl < model_->num_labels(); ++lbl)
        table.probability(0, lbl, scorer_.state(0, lbl));

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        for (label_id lbl{0}; lbl < model_->num_labels(); ++lbl)
        {
            double max_score = std::numeric_limits<double>::min();
            for (label_id in{0}; in < model_->num_labels(); ++in)
            {
                auto score = table.probability(t - 1, in)
                             + scorer_.trans(lbl, in);

                if (score > max_score)
                {
                    max_score = score;
                    table.previous_tag(t, lbl, in);
                }
            }
            table.probability(t, lbl, max_score + scorer_.state(t, lbl));
        }
    }
    return table;
}

crf::crf(const std::string& prefix)
    : scale_{1}, prefix_{prefix}
{
    filesystem::make_directory(prefix_);
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
        auto time = common::time<std::chrono::milliseconds>([&]()
        {
            loss = epoch(params, progress, iter - 1, indices, examples, scorer);
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

double crf::iteration(parameters params, uint64_t iter,
                      const sequence& seq, scorer& scorer)
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
                obs_weight(idx) += gain * pair.second
                                   * scr.state_marginal(t, lbl);
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

auto crf::make_tagger() const -> tagger
{
    return tagger{*this};
}

crf::tagger::tagger(const crf& model)
    : scorer_{model}, num_labels_{model.num_labels()}
{
    // nothing
}

void crf::tagger::tag(sequence& seq)
{
    auto trellis = scorer_.viterbi(seq);

    auto lbls = util::range<label_id>(0, num_labels_ - 1);
    auto last_lbl = functional::argmax(lbls.begin(), lbls.end(),
                                      [&](label_id lbl)
    {
        return trellis.probability(seq.size() - 1, lbl);
    });

    seq[seq.size() - 1].label(*last_lbl);
    for (uint64_t t = seq.size() - 1; t > 0; t--)
        seq[t - 1].label(trellis.previous_tag(t, seq[t].label()));
}

}
}
