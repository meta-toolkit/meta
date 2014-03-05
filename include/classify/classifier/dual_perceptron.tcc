#include "classify/classifier/dual_perceptron.h"

#include <numeric>
#include <random>

#include "util/functional.h"
#include "util/printing.h"
#include "util/progress.h"

namespace meta {
namespace classify {

template <class Kernel>
dual_perceptron<Kernel>::dual_perceptron(
    std::shared_ptr<index::forward_index> idx, Kernel&& kernel_fn, double alpha,
    double gamma, double bias, uint64_t max_iter)
    : classifier{std::move(idx)},
      alpha_{alpha},
      gamma_{gamma},
      bias_{bias},
      max_iter_{max_iter}
{
    std::function<double(pdata, pdata)> fun = [=](const pdata& a,
                                                  const pdata& b)
    {
        return kernel_fn(a, b);
    };
    kernel_ = functional::memoize(fun);
}

template <class Kernel>
void dual_perceptron<Kernel>::train(const std::vector<doc_id> & docs) {
    weights_ = {};
    for(const auto & d_id : docs)
        weights_[_idx->label(d_id)] = {};

    std::vector<uint64_t> indices(docs.size());
    std::iota(begin(indices), end(indices), 0);
    std::random_device d;
    std::mt19937 g{d()};
    for(uint64_t iter = 0; iter < max_iter_; ++iter) {
        std::shuffle(begin(indices), end(indices), g);
        uint64_t error_count = 0;
        std::stringstream ss;
        ss << " > iteration " << iter << ": ";
        printing::progress progress{ss.str(), docs.size()};
        uint64_t doc = 0;
        for(const auto & i : indices) {
            progress(doc++);
            auto guess = classify(docs[i]);
            auto actual = _idx->label(docs[i]);
            if(guess != actual) {
                ++error_count;
                decrease_weight(guess, docs[i]);
                weights_[actual][docs[i]]++;
            }
        }
        if(static_cast<double>(error_count) / docs.size() < gamma_ )
            break;
    }
}

template <class Kernel>
void dual_perceptron<Kernel>::decrease_weight(const class_label & label,
                                              const doc_id & d_id) {
    auto it = weights_[label].find(d_id);
    if(it == weights_[label].end())
        return;
    --it->second;
    if(it->second == 0)
        weights_[label].erase(it);
}

template <class Kernel>
class_label dual_perceptron<Kernel>::classify(doc_id d_id) {
    auto doc = _idx->search_primary(d_id);
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for(const auto & w : weights_) {
        double dot = 0;
        for(const auto & mistakes : w.second) {
            dot += mistakes.second
                   * (kernel_(doc, _idx->search_primary(mistakes.first))
                      + bias_);
        }
        dot *= alpha_;
        if(dot > best_dot) {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

template <class Kernel>
void dual_perceptron<Kernel>::reset() {
    weights_ = {};
}

template <class Kernel, class... Args>
dual_perceptron<Kernel> make_perceptron(index::forward_index & idx,
                                        Kernel && kernel,
                                        Args &&... args) {
    return dual_perceptron<Kernel>{idx,
                                   std::forward<Kernel>(kernel),
                                   std::forward<Args>(args)...};
}

}
}
