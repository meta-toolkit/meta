/**
 * @file winnow.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <numeric>
#include <random>
#include "cpptoml.h"
#include "meta/index/postings_data.h"
#include "meta/classify/classifier/winnow.h"

namespace meta
{
namespace classify
{

const util::string_view winnow::id = "winnow";
constexpr const double winnow::default_m;
constexpr const double winnow::default_gamma;
constexpr const size_t winnow::default_max_iter;

winnow::winnow(multiclass_dataset_view docs, double m, double gamma,
               size_t max_iter)
    : m_{m}, gamma_{gamma}, max_iter_{max_iter}
{
    zero_weights(docs);
    for (size_t iter = 0; iter < max_iter_; ++iter)
    {
        docs.shuffle();
        double error_count = 0;
        for (const auto& instance : docs)
        {
            class_label guess = classify(instance.weights);
            class_label actual = docs.label(instance);
            if (guess != actual)
            {
                error_count += 1;
                for (const auto& count : instance.weights)
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

winnow::winnow(std::istream& in)
    : m_{io::packed::read<double>(in)},
      gamma_{io::packed::read<double>(in)},
      max_iter_{io::packed::read<std::size_t>(in)}
{
    auto size = io::packed::read<std::size_t>(in);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto lbl = io::packed::read<class_label>(in);
        auto& map_ref = weights_[lbl];

        auto isize = io::packed::read<std::size_t>(in);
        for (std::size_t j = 0; j < isize; ++j)
        {
            auto id = io::packed::read<term_id>(in);
            auto weight = io::packed::read<double>(in);
            map_ref.emplace(id, weight);
        }
    }
}

void winnow::save(std::ostream& out) const
{
    io::packed::write(out, id);

    io::packed::write(out, m_);
    io::packed::write(out, gamma_);
    io::packed::write(out, max_iter_);

    io::packed::write(out, weights_.size());
    for (const auto& pr : weights_)
    {
        io::packed::write(out, pr.first);
        io::packed::write(out, pr.second.size());
        for (const auto& ipr : pr.second)
        {
            io::packed::write(out, ipr.first);
            io::packed::write(out, ipr.second);
        }
    }
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

void winnow::zero_weights(const multiclass_dataset_view& docs)
{
    for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
         ++it)
        weights_[it->first] = {};
}

class_label winnow::classify(const feature_vector& doc) const
{
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for (const auto& w : weights_)
    {
        double dot = weights_.size() / 2; // bias term
        for (const auto& count : doc)
            dot += count.second * get_weight(w.first, count.first);

        if (dot > best_dot)
        {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

template <>
std::unique_ptr<classifier>
    make_classifier<winnow>(const cpptoml::table& config,
                            multiclass_dataset_view training)
{
    auto m = config.get_as<double>("m").value_or(winnow::default_m);
    auto gamma = config.get_as<double>("gamma").value_or(winnow::default_gamma);
    auto max_iter
        = config.get_as<int64_t>("max-iter").value_or(winnow::default_max_iter);
    return make_unique<winnow>(std::move(training), m, gamma, max_iter);
}
}
}
