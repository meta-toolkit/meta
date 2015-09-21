/**
 * @file sgd.cpp
 * @author Chase Geigle
 */

#include <numeric>
#include <random>

#include "classify/classifier/sgd.h"
#include "classify/loss/loss_function_factory.h"
#include "index/postings_data.h"

namespace meta
{
namespace classify
{

const util::string_view sgd::id = "sgd";
const constexpr double sgd::default_alpha;
const constexpr double sgd::default_gamma;
const constexpr double sgd::default_bias;
const constexpr double sgd::default_lambda;
const constexpr size_t sgd::default_max_iter;

sgd::sgd(binary_dataset_view docs, std::unique_ptr<loss::loss_function> loss,
         double alpha, double gamma, double bias, double lambda,
         size_t max_iter)
    : weights_(docs.total_features()),
      alpha_{alpha},
      gamma_{gamma},
      bias_{0},
      bias_weight_{bias},
      lambda_{lambda},
      max_iter_{max_iter},
      loss_{std::move(loss)}
{
    train(std::move(docs));
}

sgd::sgd(std::istream& in)
    : alpha_{io::packed::read<double>(in)},
      gamma_{io::packed::read<double>(in)},
      bias_weight_{io::packed::read<double>(in)},
      lambda_{io::packed::read<double>(in)},
      max_iter_{io::packed::read<std::size_t>(in)}
{
    auto size = io::packed::read<std::size_t>(in);
    weights_.resize(size);
    for (std::size_t i = 0; i < size; ++i)
        io::packed::read(in, weights_[i]);

    io::packed::read(in, coeff_);
    io::packed::read(in, bias_);
    loss_ = loss::load_loss_function(in);
}

void sgd::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, alpha_);
    io::packed::write(out, gamma_);
    io::packed::write(out, bias_weight_);
    io::packed::write(out, lambda_);
    io::packed::write(out, max_iter_);

    io::packed::write(out, weights_.size());
    for (const auto& w : weights_)
        io::packed::write(out, w);

    io::packed::write(out, coeff_);
    io::packed::write(out, bias_);
    loss_->save(out);
}

void sgd::train(binary_dataset_view docs)
{
    size_t t = 0;
    double sum_loss = 0;
    double prev_sum_loss = std::numeric_limits<double>::max();
    for (size_t iter = 0; iter < max_iter_; ++iter)
    {
        docs.shuffle();
        for (const auto& instance : docs)
        {
            t += 1;

            // check for convergence every 10th of the dataset
            if (t % (docs.size() / 10) == 0)
            {
                sum_loss /= docs.size() / 10;
                if (std::abs(prev_sum_loss - sum_loss) < gamma_)
                    return;
                prev_sum_loss = sum_loss;
                sum_loss = 0;
            }

            sum_loss += train_instance(instance.weights, docs.label(instance));
        }
    }
}

void sgd::train_one(const feature_vector& doc, bool label)
{
    train_instance(doc, label);
}

double sgd::train_instance(const feature_vector& doc, bool label)
{
    // get output prediction
    // this is the binary case where p is either +1 or -1
    double prediction = predict(doc);
    int actual = label ? +1 : -1;

    auto loss = loss_->loss(prediction, actual);

    double error_derivative = loss_->derivative(prediction, actual);
    coeff_ *= (1 - alpha_ * lambda_);

    // renormalize vector of coefficient is too small
    if (coeff_ < 1e-9)
    {
        bias_ *= coeff_;
        for (auto& w : weights_)
            w *= coeff_;
        coeff_ = 1;
    }

    double update = -alpha_ * error_derivative / coeff_;
    if (update != 0)
    {
        for (const auto& count : doc)
            weights_[count.first] += update * count.second;
        bias_ += update * bias_weight_;
    }

    return loss;
}

double sgd::predict(const feature_vector& doc) const
{
    double dot = coeff_ * bias_ * bias_weight_;
    for (const auto& count : doc)
        dot += coeff_ * count.second * weights_[count.first];
    return dot;
}

template <>
std::unique_ptr<binary_classifier>
    make_binary_classifier<sgd>(const cpptoml::table& config,
                                binary_dataset_view training)
{
    auto loss = config.get_as<std::string>("loss");
    if (!loss)
        throw binary_classifier_factory::exception{
            "loss function must be specified for sgd in config"};

    auto alpha = config.get_as<double>("alpha").value_or(sgd::default_alpha);
    auto gamma = config.get_as<double>("gamma").value_or(sgd::default_gamma);
    auto bias = config.get_as<double>("bias").value_or(sgd::default_bias);
    auto lambda = config.get_as<double>("lambda").value_or(sgd::default_lambda);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(sgd::default_max_iter);

    return make_unique<sgd>(std::move(training),
                            loss::make_loss_function(*loss), alpha, gamma, bias,
                            lambda, max_iter);
}
}
}
