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

sgd::sgd(const std::string& prefix, std::shared_ptr<index::forward_index> idx,
         class_label positive, class_label negative,
         std::unique_ptr<loss::loss_function> loss, double alpha, double gamma,
         double bias, double lambda, size_t max_iter)
    : binary_classifier{std::move(idx), positive, negative},
      weights_{prefix + "_" + std::to_string(idx_->id(positive)) + ".model",
               idx_->unique_terms()},
      alpha_{alpha},
      gamma_{gamma},
      bias_{0},
      bias_weight_{bias},
      lambda_{lambda},
      max_iter_{max_iter},
      loss_{std::move(loss)}
{
    reset();
}

double sgd::predict(doc_id d_id) const
{
    auto pdata = idx_->search_primary(d_id);
    return predict(pdata->counts());
}

double sgd::predict(const counts_t& doc) const
{
    double dot = coeff_ * bias_ * bias_weight_;
    for (const auto& count : doc)
        dot += coeff_ * count.second * weights_[count.first];
    return dot;
}

void sgd::train(const std::vector<doc_id>& docs)
{
    std::vector<size_t> indices(docs.size());
    std::vector<int> labels(docs.size());
    for (size_t i = 0; i < docs.size(); ++i)
    {
        indices[i] = i;
        labels[i] = idx_->label(docs[i]) == positive_label() ? 1 : -1;
    }
    std::random_device d;
    std::mt19937 g{d()};
    size_t t = 0;
    double sum_loss = 0;
    double prev_sum_loss = std::numeric_limits<double>::max();
    for (size_t iter = 0; iter < max_iter_; ++iter)
    {
        std::shuffle(indices.begin(), indices.end(), g);
        for (size_t i = 0; i < indices.size(); ++i)
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

            auto pdata = idx_->search_primary(docs[indices[i]]);
            const counts_t& doc = pdata->counts();

            // get output prediction
            // this is the binary case where p is either +1 or -1
            double prediction = predict(doc);
            int actual = labels[indices[i]];

            sum_loss += loss_->loss(prediction, actual);

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
                {
                    weights_[count.first] += update * count.second;
                }
                bias_ += update * bias_weight_;
            }
        }
    }
}

void sgd::reset()
{
    for (auto& w : weights_)
        w = 0;
    coeff_ = 1;
    bias_ = 0;
}

template <>
std::unique_ptr<binary_classifier>
    make_binary_classifier<sgd>(const cpptoml::table& config,
                                std::shared_ptr<index::forward_index> idx,
                                class_label positive, class_label negative)
{
    auto loss = config.get_as<std::string>("loss");
    if (!loss)
        throw binary_classifier_factory::exception{
            "loss function must be specified for sgd in config"};

    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw binary_classifier_factory::exception{
            "prefix must be specified for sgd in config"};

    auto alpha = config.get_as<double>("alpha").value_or(sgd::default_alpha);
    auto gamma = config.get_as<double>("gamma").value_or(sgd::default_gamma);
    auto bias = config.get_as<double>("bias").value_or(sgd::default_bias);
    auto lambda = config.get_as<double>("lambda").value_or(sgd::default_lambda);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(sgd::default_max_iter);

    return make_unique<sgd>(
        *prefix, std::move(idx), std::move(positive), std::move(negative),
        loss::make_loss_function(*loss), alpha, gamma, bias, lambda, max_iter);
}
}
}
