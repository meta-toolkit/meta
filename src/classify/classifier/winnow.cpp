/**
 * @file winnow.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <numeric>
#include <random>
#include "cpptoml.h"
#include "index/postings_data.h"
#include "classify/classifier/winnow.h"

namespace meta
{
namespace classify
{

const std::string winnow::id = "winnow";

winnow::winnow(std::shared_ptr<index::forward_index> idx, double m,
               double gamma, size_t max_iter)
    : classifier{std::move(idx)}, m_{m}, gamma_{gamma}, max_iter_{max_iter}
{
    /* nothing */
}

double winnow::get_weight(const class_label& label, const term_id& term) const
{
    auto weight_it = weights_.find(label);
    if (weight_it == weights_.end())
        return 1;
    auto term_it = weight_it->second.find(term);
    if (term_it == weight_it->second.end())
        return 1;
    return term_it->second;
}

void winnow::zero_weights(const std::vector<doc_id>& docs)
{
    for (const auto& d_id : docs)
        weights_[idx_->label(d_id)] = {};
}

void winnow::train(const std::vector<doc_id>& docs)
{
    zero_weights(docs);
    std::vector<uint64_t> indices(docs.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::random_device d;
    std::mt19937 g{d()};

    for (size_t iter = 0; iter < max_iter_; ++iter)
    {
        std::shuffle(indices.begin(), indices.end(), g);
        double error_count = 0;
        for (size_t i = 0; i < indices.size(); ++i)
        {
            const doc_id doc{docs[indices[i]]};
            class_label guess = classify(doc);
            class_label actual = idx_->label(doc);
            if (guess != actual)
            {
                error_count += 1;
                auto pdata = idx_->search_primary(doc);
                for (const auto& count : pdata->counts())
                {
                    double guess_weight = get_weight(guess, count.first);
                    weights_[guess][count.first] = guess_weight / m_;
                    double actual_weight = get_weight(actual, count.first);
                    weights_[actual][count.first] = actual_weight * m_;
                }
            }
        }
        if (error_count / docs.size() < gamma_)
            break;
    }
}

class_label winnow::classify(doc_id d_id)
{
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for (const auto& w : weights_)
    {
        double dot = weights_.size() / 2; // bias term
        auto pdata = idx_->search_primary(d_id);
        for (const auto& count : pdata->counts())
            dot += count.second * get_weight(w.first, count.first);

        if (dot > best_dot)
        {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

void winnow::reset()
{
    weights_ = {};
}

template <>
std::unique_ptr<classifier>
    make_classifier<winnow>(const cpptoml::table& config,
                            std::shared_ptr<index::forward_index> idx)
{
    auto m = winnow::default_m;
    if (auto c_m = config.get_as<double>("m"))
        m = *c_m;

    auto gamma = winnow::default_gamma;
    if (auto c_gamma = config.get_as<double>("gamma"))
        gamma = *c_gamma;

    auto max_iter = winnow::default_max_iter;
    if (auto c_max_iter = config.get_as<int64_t>("max-iter"))
        max_iter = *c_max_iter;

    return make_unique<winnow>(std::move(idx), m, gamma, max_iter);
}
}
}
