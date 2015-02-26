/**
 * @file dual_perceptron.cpp
 * @author Chase Geigle
 */

#include <numeric>
#include <random>

#include "classify/kernel/all.h"
#include "classify/classifier/dual_perceptron.h"
#include "index/postings_data.h"
#include "util/functional.h"
#include "util/printing.h"
#include "util/progress.h"
#include "utf/utf.h"

namespace meta
{
namespace classify
{

const std::string dual_perceptron::id = "dual-perceptron";

void dual_perceptron::train(const std::vector<doc_id>& docs)
{
    weights_ = {};
    for (const auto& d_id : docs)
        weights_[idx_->label(d_id)] = {};

    std::vector<uint64_t> indices(docs.size());
    std::iota(begin(indices), end(indices), 0);
    std::random_device d;
    std::mt19937 g{d()};
    for (uint64_t iter = 0; iter < max_iter_; ++iter)
    {
        std::shuffle(begin(indices), end(indices), g);
        uint64_t error_count = 0;
        std::stringstream ss;
        ss << " > iteration " << iter << ": ";
        printing::progress progress{ss.str(), docs.size()};
        uint64_t doc = 0;
        for (const auto& i : indices)
        {
            progress(doc++);
            auto guess = classify(docs[i]);
            auto actual = idx_->label(docs[i]);
            if (guess != actual)
            {
                ++error_count;
                decrease_weight(guess, docs[i]);
                weights_[actual][docs[i]]++;
            }
        }
        if (static_cast<double>(error_count) / docs.size() < gamma_)
            break;
    }
}

void dual_perceptron::decrease_weight(const class_label& label,
                                      const doc_id& d_id)
{
    auto it = weights_[label].find(d_id);
    if (it == weights_[label].end())
        return;
    --it->second;
    if (it->second == 0)
        weights_[label].erase(it);
}

class_label dual_perceptron::classify(doc_id d_id)
{
    auto doc = idx_->search_primary(d_id);
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for (const auto& w : weights_)
    {
        double dot = 0;
        for (const auto& mistakes : w.second)
        {
            dot += mistakes.second
                   * (kernel_(doc, idx_->search_primary(mistakes.first))
                      + bias_);
        }
        dot *= alpha_;
        if (dot > best_dot)
        {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

void dual_perceptron::reset()
{
    weights_ = {};
}

template <>
std::unique_ptr<classifier>
    make_classifier<dual_perceptron>(const cpptoml::table& config,
                                     std::shared_ptr<index::forward_index> idx)
{
    auto alpha = dual_perceptron::default_alpha;
    if (auto c_alpha = config.get_as<double>("alpha"))
        alpha = *c_alpha;

    auto gamma = dual_perceptron::default_gamma;
    if (auto c_gamma = config.get_as<double>("gamma"))
        gamma = *c_gamma;

    auto bias = dual_perceptron::default_bias;
    if (auto c_bias = config.get_as<double>("bias"))
        bias = *c_bias;

    auto max_iter = dual_perceptron::default_max_iter;
    if (auto c_max_iter = config.get_as<int64_t>("max-iter"))
        max_iter = *c_max_iter;

    auto kernel = config.get_as<std::string>("kernel");
    if (!kernel)
        return make_unique<dual_perceptron>(
            std::move(idx), kernel::polynomial{}, alpha, gamma, bias, max_iter);

    auto kern = utf::tolower(*kernel);
    if (kern == "polynomial")
        return make_unique<dual_perceptron>(
            std::move(idx), kernel::polynomial{}, alpha, gamma, bias, max_iter);

    if (kern == "rbf")
    {
        auto rbf_gamma = config.get_as<double>("rbf-gamma");
        if (!rbf_gamma)
            throw classifier_factory::exception{
                "rbf kernel requires rbf-gamma in configuration"};
        return make_unique<dual_perceptron>(std::move(idx),
                                            kernel::radial_basis{*rbf_gamma},
                                            alpha, gamma, bias, max_iter);
    }

    if (kern == "sigmoid")
    {
        auto sigmoid_alpha = config.get_as<double>("sigmoid-alpha");
        if (!sigmoid_alpha)
            throw classifier_factory::exception{
                "sigmoid kernel requires sigmoid-alpha in configuration"};

        auto sigmoid_c = config.get_as<double>("sigmoid-c");
        if (!sigmoid_c)
            throw classifier_factory::exception{
                "sigmoid kernel requires sigmoid-c in configuration"};

        return make_unique<dual_perceptron>(
            std::move(idx), kernel::sigmoid{*sigmoid_alpha, *sigmoid_c}, alpha,
            gamma, bias, max_iter);
    }

    throw classifier_factory::exception{
        "dual perceptron requires kernel in configuration"};
}
}
}
