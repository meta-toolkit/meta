/**
 * @file feature_selector.cpp
 * @author Sean Massung
 * @author Siddharth Shukramani
 */

#include "meta/features/feature_selector.h"
#include "meta/io/packed.h"
#include "meta/parallel/parallel_for.h"
#include <fstream>
#include <iostream>

namespace meta
{
namespace features
{
feature_selector::feature_selector(const std::string& prefix,
                                   uint64_t total_labels,
                                   uint64_t total_features)
    : prefix_{prefix},
      total_labels_{total_labels},
      total_features_{total_features},
      selected_{prefix_ + ".selected", total_features_}
{
    // nothing
}

void feature_selector::score_all()
{
    using pair_t = std::pair<term_id, double>;
    using pair_c = std::pair<uint64_t, std::vector<pair_t>>;

    std::vector<pair_c> scores;
    std::vector<class_label> labels(total_labels_);

    uint64_t lbl_id = 0;
    uint64_t num_processed = 0;

    printing::progress prog{" > Calculating feature scores: ", total_features_};

    class_prob_.each_seen_event([&](const class_label& lbl) {
        std::vector<pair_t> class_scores;

        term_prob_.each_seen_event([&](const term_id& tid) {
            class_scores.push_back(std::make_pair(tid, score(lbl, tid)));

            prog(++num_processed);
        });

        scores.push_back(std::make_pair(lbl_id, class_scores));

        // map lbl_id to lbl
        labels[lbl_id++] = lbl;
    });

    prog.end();

    parallel::parallel_for(scores.begin(), scores.end(), [&](pair_c& c_scores) {
        std::sort(c_scores.second.begin(), c_scores.second.end(),
                  [](const pair_t& a, const pair_t& b) {
                      return a.second > b.second;
                  });

        std::ofstream out{prefix_ + "." + std::to_string(c_scores.first + 1),
                          std::ios::binary};

        io::packed::write(out, labels[c_scores.first]);

        for (auto& score : c_scores.second)
        {
            io::packed::write(out, score.first);
            io::packed::write(out, score.second);
        }
    });
}

void feature_selector::select(uint64_t features_per_class /* = 20 */)
{
    if ((total_labels_ * features_per_class) > total_features_)
        throw feature_selector_exception{"cannot select more than the total "
                                         "number of features in the dataset"};

    // zero out old vector
    for (auto& b : selected_)
        b = false;

    term_id tid;
    double score;

    printing::progress prog{" > Selecting " + std::to_string(features_per_class)
                                + " features per class: ",
                            (total_labels_ * features_per_class)};

    for (uint64_t lbl_id = 0; lbl_id < total_labels_; ++lbl_id)
    {
        std::ifstream in{prefix_ + "." + std::to_string(lbl_id + 1),
                         std::ios::binary};

        // read the label first
        std::string lbl;
        io::packed::read(in, lbl);

        for (uint64_t i = 0; i < features_per_class; ++i)
        {
            io::packed::read(in, tid);
            io::packed::read(in, score);

            selected_[tid] = true;

            prog((lbl_id + 1) * (i + 1));
        }

        prog.end();
    };
}

bool feature_selector::selected(term_id tid) const
{
    return selected_[tid];
}

void feature_selector::select_percent(double p /* = 0.05 */)
{
    if (p <= 0.0 || p >= 1.0)
        throw feature_selector_exception{
            "select_percent needs a value p, 0 < p < 1"};

    double num_features = p * total_features_;
    // truncate to int
    auto per_class = static_cast<uint64_t>(num_features / total_labels_);

    select(per_class);
}

void feature_selector::print_summary(std::shared_ptr<index::disk_index> idx,
                                     uint64_t k /* = 20 */) const
{
    term_id tid;
    double score;

    for (uint64_t lbl_id = 0; lbl_id < total_labels_; ++lbl_id)
    {
        // read (term_id, score) pairs
        std::ifstream in{prefix_ + "." + std::to_string(lbl_id + 1),
                         std::ios::binary};

        std::string lbl;
        io::packed::read(in, lbl);

        std::cout << std::endl
                  << "Top " << k << " features for \"" << lbl
                  << "\":" << std::endl
                  << "===============================" << std::endl;

        for (uint64_t i = 0; i < k; ++i)
        {
            io::packed::read(in, tid);
            io::packed::read(in, score);
            std::cout << (i + 1) << ". " << idx->term_text(tid) << " (" << score
                      << ")" << std::endl;
        }
    }
}

double feature_selector::prob_term(term_id tid) const
{
    auto p = term_prob_.probability(tid);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::prob_class(const class_label& lbl) const
{
    auto p = class_prob_.probability(lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::term_and_class(term_id tid,
                                        const class_label& lbl) const
{
    auto p = co_occur_.probability(std::make_pair(lbl, tid));
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::not_term_and_not_class(term_id tid,
                                                const class_label& lbl) const
{
    auto p = 1.0 - term_and_class(tid, lbl) - not_term_and_class(tid, lbl)
             - term_and_not_class(tid, lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::term_and_not_class(term_id tid,
                                            const class_label& lbl) const
{
    auto p = term_prob_.probability(tid) - term_and_class(tid, lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::not_term_and_class(term_id tid,
                                            const class_label& lbl) const
{
    auto p = class_prob_.probability(lbl) - term_and_class(tid, lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}
}
}
