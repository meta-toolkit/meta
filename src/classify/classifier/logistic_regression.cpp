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

logistic_regression::logistic_regression(multiclass_dataset_view docs,
                                         double alpha, double gamma,
                                         double bias, double lambda,
                                         uint64_t max_iter)
{
    using size_type = multiclass_dataset_view::size_type;
    using indices_type = std::vector<size_type>;

    pivot_ = docs.label(*docs.begin());

    for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
         ++it)
        classifiers_[it->first] = nullptr;

    std::unordered_map<class_label, indices_type> docs_by_class;
    for (auto it = docs.begin(), end = docs.end(); it != end; ++it)
        docs_by_class[docs.label(*it)].push_back(it.index());

    using T = decltype(*classifiers_.begin());
    parallel::parallel_for(
        classifiers_.begin(), classifiers_.end(), [&](T& pair)
        {
            auto train_docs = docs_by_class[pair.first];
            auto pivot_docs = docs_by_class[pivot_];
            train_docs.insert(train_docs.end(), pivot_docs.begin(),
                              pivot_docs.end());

            binary_dataset_view bdv{
                docs, std::move(train_docs), [&](const instance_type& instance)
                {
                    return docs.label(instance) == pair.first;
                }};

            pair.second = make_unique<sgd>(
                bdv, loss::make_loss_function<loss::logistic>(), alpha, gamma,
                bias, lambda, max_iter);
        });
}

logistic_regression::logistic_regression(std::istream& in)
{
    auto size = io::packed::read<std::size_t>(in);
    classifiers_.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto lbl = io::packed::read<class_label>(in);
        classifiers_[lbl] = load_binary_classifier(in);
    }
    io::packed::read(in, pivot_);
}

void logistic_regression::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, classifiers_.size());
    for (const auto& pr : classifiers_)
    {
        io::packed::write(out, pr.first);
        pr.second->save(out);
    }
    io::packed::write(out, pivot_);
}

std::unordered_map<class_label, double>
    logistic_regression::predict(const feature_vector& doc) const
{
    std::unordered_map<class_label, double> probs;
    double denom = 0;
    for (auto& pair : classifiers_)
    {
        auto prediction = std::exp(pair.second->predict(doc));
        probs[pair.first] = prediction;
        denom += prediction;
    }
    probs[pivot_] = 1;
    denom += 1;
    for (auto& pair : probs)
        pair.second /= denom;
    return probs;
}

class_label logistic_regression::classify(const feature_vector& doc) const
{
    using namespace functional;
    auto probs = predict(doc);
    auto it = argmax(probs.begin(), probs.end(),
                     [](const std::pair<class_label, double>& pair)
                     {
                         return pair.second;
                     });
    return it->first;
}

template <>
std::unique_ptr<classifier>
    make_classifier<logistic_regression>(const cpptoml::table& config,
                                         multiclass_dataset_view training)
{
    auto alpha = config.get_as<double>("alpha").value_or(sgd::default_alpha);
    auto gamma = config.get_as<double>("gamma").value_or(sgd::default_gamma);
    auto bias = config.get_as<double>("bias").value_or(sgd::default_bias);
    auto lambda = config.get_as<double>("lambda").value_or(sgd::default_lambda);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(sgd::default_max_iter);

    return make_unique<logistic_regression>(std::move(training), alpha, gamma,
                                            bias, lambda, max_iter);
}
}
}
