/**
 * @file sgd.cpp
 * @author Chase Geigle
 */

#include "meta/io/packed.h"
#include "meta/learn/sgd.h"
#include <cmath>
#include <numeric>

namespace meta
{
namespace learn
{

const constexpr double sgd_model::default_learning_rate;
const constexpr double sgd_model::default_l2_regularizer;
const constexpr double sgd_model::default_l1_regularizer;

sgd_model::sgd_model(std::size_t num_features, options_type options)
    : weights_(num_features),
      scale_{1.0},
      update_scale_{0.0},
      lr_{options.learning_rate},
      l2_regularization_{options.l2_regularizer},
      l1_regularization_{options.l1_regularizer},
      t_{0}
{
    // nothing
}

sgd_model::sgd_model(std::istream& in)
{
    auto size = io::packed::read<std::size_t>(in);

    weights_.resize(size);
    for (auto& weight_val : weights_)
    {
        io::packed::read(in, weight_val.weight);
        io::packed::read(in, weight_val.grad_squared);
        io::packed::read(in, weight_val.cumulative_penalty);
    }
    io::packed::read(in, bias_.weight);
    io::packed::read(in, bias_.grad_squared);
    io::packed::read(in, bias_.cumulative_penalty);
    io::packed::read(in, scale_);
    io::packed::read(in, update_scale_);
    io::packed::read(in, lr_);
    io::packed::read(in, l2_regularization_);
    io::packed::read(in, l1_regularization_);
    io::packed::read(in, t_);
}

void sgd_model::save(std::ostream& out) const
{
    io::packed::write(out, weights_.size());
    for (const auto& weight_val : weights_)
    {
        io::packed::write(out, weight_val.weight);
        io::packed::write(out, weight_val.grad_squared);
        io::packed::write(out, weight_val.cumulative_penalty);
    }
    io::packed::write(out, bias_.weight);
    io::packed::write(out, bias_.grad_squared);
    io::packed::write(out, bias_.cumulative_penalty);
    io::packed::write(out, scale_);
    io::packed::write(out, update_scale_);
    io::packed::write(out, lr_);
    io::packed::write(out, l2_regularization_);
    io::packed::write(out, l1_regularization_);
    io::packed::write(out, t_);
}

double sgd_model::predict(const feature_vector& x) const
{
    auto val = scale_ * bias_.weight;
    for (const auto& pr : x)
        val += pr.second * scale_ * weights_.at(pr.first).weight;
    return val;
}

double sgd_model::train_one(const feature_vector& x, double expected_label,
                            const loss::loss_function& loss)
{

    const auto predicted = predict(x);
    const auto error_derivative = loss.derivative(predicted, expected_label);
    const auto reg = l2_regularization_ + l1_regularization_;
    const auto eta = lr_ / (1.0 + reg * lr_ * t_);
    t_ += 1;

    scale_ *= (1.0 - eta * l2_regularization_);

    // renormalize if scalar is too small
    if (scale_ < 1e-10)
    {
        for (auto& weight_val : weights_)
            weight_val.weight *= scale_;
        bias_.weight *= scale_;
        scale_ = 1;
    }

    if (error_derivative != 0.0)
    {
        for (const auto& pr : x)
        {
            if (pr.second == 0.0)
                continue;

            auto& w = weights_[pr.first];
            const auto gt = error_derivative * pr.second;
            w.grad_squared += gt * gt;
            w.weight -= eta / scale_ * gt / std::sqrt(w.grad_squared);

            // handle the L1 penalization
            if (l1_regularization_ > 0)
                penalize(w);
        }

        // handle the bias (we treat it as always being 1)
        const auto gt = error_derivative;
        bias_.grad_squared += gt * gt;
        bias_.weight -= eta / scale_ * gt / std::sqrt(bias_.grad_squared);
    }

    return loss.loss(predicted, expected_label);
}

void sgd_model::penalize(weight_type& weight_val)
{
    auto u = t_ * lr_ * l1_regularization_;
    auto z = weight_val.weight * scale_;
    if (z > 0)
    {
        weight_val.weight
            = std::max(0.0, z - (u + weight_val.cumulative_penalty)) / scale_;
    }
    else
    {
        weight_val.weight
            = std::min(0.0, z + (u - weight_val.cumulative_penalty)) / scale_;
    }
    weight_val.cumulative_penalty += (scale_ * weight_val.weight) - z;
}

void sgd_model::reset()
{
    std::fill(weights_.begin(), weights_.end(), weight_type{});
    bias_ = weight_type{};
    scale_ = 1;
    update_scale_ = 0;
    t_ = 0;
}

double sgd_model::l2norm() const
{
    auto norm = 0.0;
    for (const auto& w : weights_)
        norm += w.weight * w.weight;
    norm += bias_.weight * bias_.weight;
    return std::sqrt(norm * scale_ * scale_);
}

double sgd_model::l1norm() const
{
    return scale_ * std::accumulate(weights_.begin(), weights_.end(),
                                    bias_.weight,
                                    [&](double accum, const weight_type& w) {
                                        return accum + std::abs(w.weight);
                                    });
}
}
}
