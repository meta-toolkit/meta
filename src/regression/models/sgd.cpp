/**
 * @file sgd.cpp
 * @author Chase Geigle
 */

#include "meta/regression/models/sgd.h"
#include "meta/learn/loss/loss_function_factory.h"

namespace meta
{
namespace regression
{

const util::string_view sgd::id = "sgd";
const constexpr double sgd::default_gamma;
const constexpr size_t sgd::default_max_iter;

sgd::sgd(dataset_view_type docs,
         std::unique_ptr<learn::loss::loss_function> loss,
         learn::sgd_model::options_type options, double gamma, size_t max_iter,
         bool calibrate)
    : model_{docs.total_features(), options},
      gamma_{gamma},
      max_iter_{max_iter},
      loss_{std::move(loss)}
{
    if (calibrate)
    {
        model_.calibrate(docs, *loss_, [&](const learn::instance& inst)
                         {
                             return docs.label(inst);
                         });
    }
    train(std::move(docs));
}

sgd::sgd(std::istream& in)
    : model_{[&]()
             {
                 return learn::sgd_model{in};
             }()},
      gamma_{io::packed::read<double>(in)},
      max_iter_{io::packed::read<std::size_t>(in)}
{
    loss_ = learn::loss::load_loss_function(in);
}

void sgd::save(std::ostream& out) const
{
    io::packed::write(out, id);

    model_.save(out);
    io::packed::write(out, gamma_);
    io::packed::write(out, max_iter_);
    loss_->save(out);
}

void sgd::train(dataset_view_type docs)
{
    double prev_total_loss = std::numeric_limits<double>::max();
    for (size_t iter = 0; iter < max_iter_; ++iter)
    {
        double total_loss = 0;
        docs.shuffle();
        for (const auto& instance : docs)
        {
            total_loss += model_.train_one(instance.weights, docs.label(instance),
                                         *loss_);
        }

        if (prev_total_loss - total_loss < gamma_ * docs.size())
            break;
        prev_total_loss = total_loss;
    }
}

void sgd::train_one(const feature_vector& doc, double label)
{
    model_.train_one(doc, label, *loss_);
}

double sgd::predict(const feature_vector& doc) const
{
    return model_.predict(doc);
}

template <>
std::unique_ptr<regressor> make_regressor<sgd>(const cpptoml::table& config,
                                               regression_dataset_view training)
{
    auto loss = config.get_as<std::string>("loss");
    if (!loss)
        throw regressor_factory::exception{
            "loss function must be specified for sgd in config"};

    learn::sgd_model::options_type options;

    if (auto alpha = config.get_as<double>("learning-rate"))
        options.learning_rate = *alpha;

    if (auto l2_lambda = config.get_as<double>("l2-regularization"))
        options.l2_regularizer = *l2_lambda;

    if (auto l1_lambda = config.get_as<double>("l1-regularization"))
        options.l1_regularizer = *l1_lambda;

    auto gamma = config.get_as<double>("convergence-threshold")
                     .value_or(sgd::default_gamma);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(sgd::default_max_iter);

    auto calibrate = config.get_as<bool>("calibrate").value_or(true);

    return make_unique<sgd>(std::move(training),
                            learn::loss::make_loss_function(*loss), options,
                            gamma, max_iter, calibrate);
}
}
}
