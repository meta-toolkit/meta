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

double crf::train(parameters params, const std::vector<sequence>& examples)
{
    std::vector<uint64_t> indices(examples.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 rng{std::random_device{}()};

    double loss = std::numeric_limits<double>::max();
    double old_sum_loss = loss;
    double sum_loss = loss;
    for (uint64_t iter = 1; iter <= params.max_iters; ++iter)
    {
        printing::progress progress{" > Epoch " + std::to_string(iter) + ": ",
                                    examples.size()};
        std::shuffle(indices.begin(), indices.end(), rng);
        loss = epoch(params, progress, indices, examples);
        sum_loss += loss;
        if (iter % params.period == 0)
        {
            if (iter > params.period)
            {
                double delta = old_sum_loss - sum_loss;
                if (delta < params.delta)
                    return loss;
            }
            old_sum_loss = sum_loss;
            sum_loss = 0;
        }
    }
    return loss;
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
    return sum_loss;
}

double crf::iteration(parameters params, const sequence& seq)
{
    scale_ *= (1 - params.lambda * params.lr);
    auto gain = params.lr / scale_;
    gradient_observation_expectation(seq, gain);
    gradient_model_expectation(seq, -1.0 * gain);
    // TODO: return value of the loss on this sequence
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

void crf::gradient_model_expectation(const sequence& seq, double gain)
{
    auto fwd = forward(seq);
    auto bwd = backward(seq, fwd);
    // TODO
}

}
}
