/**
 * @file logistic_regression.cpp
 * @author Chase Geigle
 */

#include "classify/classifier/logistic_regression.h"
#include "classify/loss/loss_function_factory.h"
#include "classify/loss/logistic.h"
#include "parallel/parallel_for.h"
#include "util/functional.h"

namespace meta
{
namespace classify
{

const util::string_view logistic_regression::id = "logistic-regression";

logistic_regression::logistic_regression(
    const std::string& prefix, std::shared_ptr<index::forward_index> idx,
    double alpha, double gamma, double bias, double lambda, uint64_t max_iter)
    : classifier{std::move(idx)}
{
    auto labels = idx_->class_labels();
    pivot_ = labels[labels.size() - 1];
    for (uint64_t i = 0; i < labels.size() - 1; ++i)
    {
        using namespace loss;
        classifiers_.emplace(
            std::piecewise_construct, std::forward_as_tuple(labels[i]),
            std::forward_as_tuple(prefix, idx_, labels[i], pivot_,
                                  make_loss_function<logistic>(), alpha, gamma,
                                  bias, lambda, max_iter));
    }
}

std::unordered_map<class_label, double>
    logistic_regression::predict(doc_id d_id)
{
    std::unordered_map<class_label, double> probs;
    double denom = 0;
    for (auto& pair : classifiers_)
    {
        auto prediction = std::exp(pair.second.predict(d_id));
        probs[pair.first] = prediction;
        denom += prediction;
    }
    probs[pivot_] = 1;
    denom += 1;
    for (auto& pair : probs)
        pair.second /= denom;
    return probs;
}

class_label logistic_regression::classify(doc_id d_id)
{
    using namespace functional;
    auto probs = predict(d_id);
    auto it = argmax(probs.begin(), probs.end(),
                     [](const std::pair<class_label, double>& pair)
                     {
                         return pair.second;
                     });
    return it->first;
}

void logistic_regression::train(const std::vector<doc_id>& docs)
{
    std::unordered_map<class_label, std::vector<doc_id>> docs_by_class;
    for (const auto& d_id : docs)
        docs_by_class[idx_->label(d_id)].emplace_back(d_id);
    using T = decltype(*classifiers_.begin());
    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(), [&](T& pair)
        {
            auto train_docs = docs_by_class[pair.first];
            auto pivot_docs = docs_by_class[pivot_];
            train_docs.insert(train_docs.end(), pivot_docs.begin(),
                              pivot_docs.end());
            pair.second.train(train_docs);
        });
}

void logistic_regression::reset()
{
    for (auto& pair : classifiers_)
        pair.second.reset();
}

template <>
std::unique_ptr<classifier> make_classifier<logistic_regression>(
    const cpptoml::table& config, std::shared_ptr<index::forward_index> idx)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw classifier_factory::exception{
            "prefix must be specified for logistic-regression in config"};

    auto alpha = config.get_as<double>("alpha").value_or(sgd::default_alpha);
    auto gamma = config.get_as<double>("gamma").value_or(sgd::default_gamma);
    auto bias = config.get_as<double>("bias").value_or(sgd::default_bias);
    auto lambda = config.get_as<double>("lambda").value_or(sgd::default_lambda);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(sgd::default_max_iter);

    return make_unique<logistic_regression>(*prefix, std::move(idx), alpha,
                                            gamma, bias, lambda, max_iter);
}
}
}
