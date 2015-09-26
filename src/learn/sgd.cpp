/**
 * @file sgd.cpp
 * @author Chase Geigle
 */

#include "io/packed.h"
#include "learn/sgd.h"

namespace meta
{
namespace learn
{

sgd_model::sgd_model(std::size_t num_features, double learning_rate,
                     double regularization)
    : weights_(num_features),
      scale_{1.0},
      update_scale_{0.0},
      lr_{learning_rate},
      regularization_{regularization},
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
        io::packed::read(in, weight_val.scale);
        io::packed::read(in, weight_val.grad_squared);
    }
    io::packed::read(in, bias_.weight);
    io::packed::read(in, bias_.grad_squared);
    io::packed::read(in, scale_);
    io::packed::read(in, update_scale_);
    io::packed::read(in, lr_);
    io::packed::read(in, regularization_);
    io::packed::read(in, t_);
}

void sgd_model::save(std::ostream& out) const
{
    io::packed::write(out, weights_.size());
    for (const auto& weight_val : weights_)
    {
        io::packed::write(out, weight_val.weight);
        io::packed::write(out, weight_val.scale);
        io::packed::write(out, weight_val.grad_squared);
    }
    io::packed::write(out, bias_.weight);
    io::packed::write(out, bias_.grad_squared);
    io::packed::write(out, scale_);
    io::packed::write(out, update_scale_);
    io::packed::write(out, lr_);
    io::packed::write(out, regularization_);
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
    t_ += 1;

    auto predicted = 0.0;
    for (const auto& pr : x)
    {
        auto abs_val = std::abs(pr.second);
        auto& weight_val = weights_.at(pr.first);
        if (abs_val > weight_val.scale)
        {
            weight_val.weight *= weight_val.scale / abs_val;
            weight_val.scale = abs_val;
        }

        if (weight_val.scale > 0)
            update_scale_ += (pr.second * pr.second)
                             / (weight_val.scale * weight_val.scale);

        predicted += pr.second * scale_ * weight_val.weight;
    }

    // handle the bias (we treat it as always being 1)
    update_scale_ += 1.0;
    predicted += scale_ * bias_.weight;

    auto error_derivative = loss.derivative(predicted, expected_label);
    scale_ *= (1.0 - lr_ * regularization_);

    // renormalize if scalar is too small
    if (scale_ < 1e-10)
    {
        for (auto& weight_val : weights_)
            weight_val.weight *= scale_;
        bias_.weight *= scale_;
        scale_ = 1;
    }

    auto delta = -lr_ * std::sqrt(t_ / update_scale_) * error_derivative
                 / scale_;
    if (delta != 0.0)
    {
        for (const auto& pr : x)
        {
            auto& weight_val = weights_.at(pr.first);
            weight_val.grad_squared += error_derivative * error_derivative
                                       * pr.second * pr.second;
            weight_val.weight
                += delta * 1.0
                   / (weight_val.scale * std::sqrt(weight_val.grad_squared))
                   * pr.second;
        }

        // handle the bias (we treat it as always being 1)
        bias_.grad_squared += error_derivative * error_derivative;
        bias_.weight += delta * 1.0 / (std::sqrt(bias_.grad_squared));
    }

    return loss.loss(predicted, expected_label);
}
}
}
