/**
 * @file dual_perceptron.cpp
 * @author Chase Geigle
 */

#include <numeric>
#include <random>

#include "meta/classify/kernel/all.h"
#include "meta/classify/classifier/dual_perceptron.h"
#include "meta/index/postings_data.h"
#include "meta/io/packed.h"
#include "meta/util/functional.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"
#include "meta/utf/utf.h"

namespace meta
{
namespace classify
{

const util::string_view dual_perceptron::id = "dual-perceptron";
const constexpr double dual_perceptron::default_alpha;
const constexpr double dual_perceptron::default_gamma;
const constexpr double dual_perceptron::default_bias;
const constexpr uint64_t dual_perceptron::default_max_iter;

dual_perceptron::dual_perceptron(multiclass_dataset_view docs,
                                 std::unique_ptr<kernel::kernel> kernel_fn,
                                 double alpha, double gamma, double bias,
                                 uint64_t max_iter)
    : kernel_{std::move(kernel_fn)},
      alpha_{alpha},
      gamma_{gamma},
      bias_{bias},
      max_iter_{max_iter}
{
    train(std::move(docs));
}

dual_perceptron::dual_perceptron(std::istream& in)
    : alpha_{io::packed::read<double>(in)},
      gamma_{io::packed::read<double>(in)},
      bias_{io::packed::read<double>(in)},
      max_iter_{io::packed::read<uint64_t>(in)}
{
    // mistake counts
    auto size = io::packed::read<std::size_t>(in);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto lbl = io::packed::read<class_label>(in);
        auto& map_ref = weights_[lbl];
        auto isize = io::packed::read<std::size_t>(in);
        for (std::size_t j = 0; j < isize; ++j)
        {
            auto id = io::packed::read<learn::instance_id>(in);
            auto weight = io::packed::read<uint64_t>(in);
            map_ref.emplace(id, weight);
        }
    }

    // support vectors
    io::packed::read(in, size);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto id = io::packed::read<learn::instance_id>(in);
        auto& map_ref = svs_[id];
        auto isize = io::packed::read<std::size_t>(in);
        for (std::size_t j = 0; j < isize; ++j)
        {
            auto fid = io::packed::read<learn::feature_id>(in);
            auto weight = io::packed::read<double>(in);
            map_ref.emplace_back(fid, weight);
        }
    }

    // kernel function
    kernel_ = kernel::load_kernel(in);
}

void dual_perceptron::save(std::ostream& out) const
{
    io::packed::write(out, id);

    // training parameters
    io::packed::write(out, alpha_);
    io::packed::write(out, gamma_);
    io::packed::write(out, bias_);
    io::packed::write(out, max_iter_);

    // mistake counts
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

    // support vectors
    io::packed::write(out, svs_.size());
    for (const auto& pr : svs_)
    {
        io::packed::write(out, pr.first);
        io::packed::write(out, pr.second.size());
        for (const auto& ipr : pr.second)
        {
            io::packed::write(out, ipr.first);
            io::packed::write(out, ipr.second);
        }
    }

    // kernel function
    kernel_->save(out);
}

void dual_perceptron::train(multiclass_dataset_view docs)
{
    for (auto it = docs.labels_begin(), end = docs.labels_end(); it != end;
         ++it)
        weights_[it->first] = {};

    for (uint64_t iter = 0; iter < max_iter_; ++iter)
    {
        docs.shuffle();
        uint64_t error_count = 0;
        std::stringstream ss;
        ss << " > iteration " << iter << ": ";
        printing::progress progress{ss.str(), docs.size()};
        uint64_t doc = 0;
        for (const auto& instance : docs)
        {
            progress(doc++);
            auto guess = classify(instance.weights);
            auto actual = docs.label(instance);
            if (guess != actual)
            {
                ++error_count;
                // memorize the training instance if we haven't already
                if (svs_.find(instance.id) == svs_.end())
                    svs_[instance.id] = instance.weights;

                decrease_weight(guess, instance.id);
                weights_[actual][instance.id]++;
            }
        }
        if (static_cast<double>(error_count) / docs.size() < gamma_)
            break;
    }
}

void dual_perceptron::decrease_weight(const class_label& label,
                                      const learn::instance_id& d_id)
{
    auto it = weights_[label].find(d_id);
    if (it == weights_[label].end())
        return;
    --it->second;
    if (it->second == 0)
        weights_[label].erase(it);
}

class_label dual_perceptron::classify(const feature_vector& doc) const
{
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for (const auto& w : weights_)
    {
        double dot = 0;
        for (const auto& mistakes : w.second)
        {
            dot += mistakes.second
                   * ((*kernel_)(doc, svs_.at(mistakes.first)) + bias_);
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



template <>
std::unique_ptr<classifier>
    make_classifier<dual_perceptron>(const cpptoml::table& config,
                                     multiclass_dataset_view training)
{
    auto alpha = config.get_as<double>("alpha")
                     .value_or(dual_perceptron::default_alpha);

    auto gamma = config.get_as<double>("gamma")
                     .value_or(dual_perceptron::default_gamma);

    auto bias
        = config.get_as<double>("bias").value_or(dual_perceptron::default_bias);

    auto max_iter = config.get_as<int64_t>("max-iter")
                        .value_or(dual_perceptron::default_max_iter);

    auto kernel_cfg = config.get_table("kernel");
    if (!kernel_cfg)
        return make_unique<dual_perceptron>(std::move(training),
                                            make_unique<kernel::polynomial>(),
                                            alpha, gamma, bias, max_iter);

    return make_unique<dual_perceptron>(std::move(training),
                                        kernel::make_kernel(*kernel_cfg), alpha,
                                        gamma, bias, max_iter);
}
}
}
