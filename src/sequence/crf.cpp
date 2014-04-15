/**
 * @file crf.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <random>
#include <numeric>
#include "sequence/crf.h"
#include "util/mapping.h"
#include "util/optional.h"
#include "util/progress.h"

namespace meta
{
namespace sequence
{

crf::crf(const std::string& prefix)
    : observation_weights_{prefix + "/observation_weights.vector"},
      transition_weights_{prefix + "/transition_weights.vector"},
      prefix_{prefix}
{
    map::load_mapping(label_id_mapping_, prefix_ + "/label.mapping");
}

crf::crf(sequence_analyzer& analyzer)
    : observation_weights_{analyzer.prefix() + "/observation_weights.vector",
                           analyzer.num_features() * analyzer.num_labels()},
      transition_weights_{analyzer.prefix() + "/transition_weights.vector",
                          analyzer.num_labels() * analyzer.num_labels()},
      prefix_{analyzer.prefix()}
{
    map::load_mapping(label_id_mapping_, prefix_ + "/label.mapping");
    reset();
}

void crf::reset()
{
    for (auto& w : observation_weights_)
        w = 0;
    for (auto& w : transition_weights_)
        w = 0;
    scale_ = 1;
}

const double& crf::weight(label_id lbl, feature_id feat) const
{
    auto num_feats = observation_weights_.size() / label_id_mapping_.size();
    return observation_weights_[lbl * num_feats + feat];
}

double& crf::weight(label_id lbl, feature_id feat)
{
    auto num_feats = observation_weights_.size() / label_id_mapping_.size();
    return observation_weights_[lbl * num_feats + feat];
}

const double& crf::weight(label_id from, label_id to) const
{
    return transition_weights_[from * label_id_mapping_.size() + to];
}

double& crf::weight(label_id from, label_id to)
{
    return transition_weights_[from * label_id_mapping_.size() + to];
}

double crf::train(parameters params, const std::vector<sequence>& examples)
{
    std::vector<uint64_t> indices(examples.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng{std::random_device{}()};

    double old_loss = std::numeric_limits<double>::max();
    double loss = old_loss;
    double delta;
    for (uint64_t iter = 1; iter <= params.max_iters; ++iter)
    {
        std::stringstream ss;
        ss << " > Epoch " << iter << " : ";
        printing::progress progress{ss.str(), examples.size()};
        progress.print_endline(false);
        std::shuffle(indices.begin(), indices.end(), rng);
        loss = epoch(params, progress, indices, examples);
        ss << " loss=" << loss;
        if (iter % params.period == 0)
        {
            if (iter > params.period)
            {
                delta = std::abs(old_loss - loss) / loss;
                ss << ", improvement=" << delta;
                if (delta < params.delta)
                {
                    ss << ", converged!";
                    LOG(progress) << "\r" << ss.str() << "\n" << ENDLG;
                    return loss;
                }
            }
        }
        LOG(progress) << "\r" << ss.str() << "\n" << ENDLG;
        old_loss = loss;
        loss = 0;
    }
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
        auto lbl = label_id_mapping_.get_value(obs.tag());
        for (const auto& pair : obs.features())
        {
            weight(lbl, pair.first) += gain * pair.second;
        }

        if (prev)
        {
            weight(*prev, lbl) += gain;
        }
    }
}

void crf::gradient_model_expectation(const sequence& seq, double gain,
                                     const double_matrix& state_mrg,
                                     const double_matrix& trans_mrg)
{
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
        {
            for (const auto& pair : seq[t].features())
            {
                weight(lbl, pair.first) +=
                    gain * pair.second * state_mrg[t][lbl];
            }
        }
    }

    for (label_id i{0}; i < label_id_mapping_.size(); ++i)
    {
        for (label_id j{0}; j < label_id_mapping_.size(); ++j)
        {
            weight(i, j) += gain * trans_mrg[i][j];
        }
    }
}

void crf::state_scores(const sequence& seq)
{
    state_.resize(seq.size());
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        state_[t].resize(label_id_mapping_.size());
        for (const auto& pair : seq[t].features())
        {
            for (label_id lbl{0}; lbl < label_id_mapping_.size(); ++lbl)
            {
                auto val = weight(lbl, pair.first) * pair.second;
                state_[t][lbl] += val;
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
        for (label_id inner{0}; inner < num_labels; ++inner)
            trans_[outer][inner] = std::exp(weight(outer, inner));
    }
}

forward_trellis crf::forward(const sequence& seq) const
{
    forward_trellis table{seq.size()};

    // initialize first column of trellis
    double normalizer = 0;
    for (const auto& lbl_pair : label_id_mapping_)
    {
        auto val = state_[0][lbl_pair.second];
        table.probability(0, lbl_pair.first, val);
        normalizer += val;
    }
    // normalize to avoid underflow
    table.normalize(0, normalizer > 0 ? 1 / normalizer : 1);

    // compute remaining columns of trellis using recursive formulation
    for (uint64_t t = 1; t < seq.size(); ++t)
    {
        double normalizer = 0;
        for (const auto& lbl_pair : label_id_mapping_)
        {
            auto score = state_[t][lbl_pair.second];
            double sum = 0;
            for (const auto& inner_pr : label_id_mapping_)
            {
                sum += table.probability(t - 1, inner_pr.first)
                       * trans_[inner_pr.second][lbl_pair.second];
            }
            score *= sum;
            table.probability(t, lbl_pair.first, score);
            normalizer += sum;
        }
        // normalize to avoid underflow
        table.normalize(t, normalizer > 0 ? 1.0 / normalizer : 1);
    }

    return table;
}

trellis crf::backward(const sequence& seq, const forward_trellis& fwd) const
{
    trellis table{seq.size()};

    // initialize last column of the trellis
    for (const auto& lbl_pair : label_id_mapping_)
    {
        auto val = fwd.normalizer(seq.size() - 1);
        table.probability(seq.size() - 1, lbl_pair.first, val);
    }

    // to avoid unsigned weirdness, t is really t+1, so we are actually
    // going to compute for index t-1 here to compute beta[t]
    for (uint64_t t = seq.size() - 1; t > 0; --t)
    {
        for (const auto& lbl_pair : label_id_mapping_)
        {
            double sum = 0;
            for (const auto& inner_pr : label_id_mapping_)
            {
                sum += state_[t][inner_pr.second]
                       * trans_[lbl_pair.second][inner_pr.second]
                       * table.probability(t, inner_pr.first);
            }
            table.probability(t - 1, lbl_pair.first,
                              sum * fwd.normalizer(t - 1));
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
        for (const auto& lbl_pair : label_id_mapping_)
        {
            table[t][lbl_pair.second] = fwd.probability(t, lbl_pair.first)
                                        * bwd.probability(t, lbl_pair.first)
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
        for (const auto& lbl_pair : label_id_mapping_)
        {
            for (const auto& inner_pr : label_id_mapping_)
            {
                table[lbl_pair.second][inner_pr.second] +=
                    fwd.probability(t, lbl_pair.first)
                    * state_[t + 1][inner_pr.second]
                    * trans_[lbl_pair.second][inner_pr.second]
                    * bwd.probability(t + 1, inner_pr.first);
            }
        }
    }
    return table;
}

double crf::loss(const sequence& seq, const forward_trellis& fwd) const
{
    auto lbl = [&](tag_t tag) { return label_id_mapping_.get_value(tag); };

    util::optional<label_id> prev;
    double result = 0;
    for (uint64_t t = 0; t < seq.size(); ++t)
    {
        auto curr = lbl(seq[t].tag());
        result += std::log(state_[t][curr]);
        if (prev)
            result += std::log(trans_[*prev][curr]);

        // factor in log normalizer: log(Z(x)) = - \sum_t log(scale[t])
        // and loss = score(x) - log(Z(x)), so we add on log(scale[t])
        result += std::log(fwd.normalizer(t));
    }
    return result;
}

double crf::l2norm() const
{
    double norm = 0;
    for (const auto& w : observation_weights_)
        norm += w * w;
    for (const auto& w : transition_weights_)
        norm += w * w;
    return norm * scale_ * scale_;
}
}
}
