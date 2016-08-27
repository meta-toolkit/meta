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
    : prefix_{[&]() {
          filesystem::make_directories(prefix);
          return prefix;
      }()},
      total_labels_{total_labels},
      total_features_{total_features}
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

        std::ofstream out{prefix_ + "/" + std::to_string(c_scores.first + 1)
                              + ".bin",
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

    term_id tid;
    double score;

    printing::progress prog{" > Selecting " + std::to_string(features_per_class)
                                + " features per class: ",
                            (total_labels_ * features_per_class)};

    std::vector<uint64_t> positions;
    positions.reserve(total_labels_ * features_per_class);
    for (uint64_t lbl_id = 0; lbl_id < total_labels_; ++lbl_id)
    {
        std::ifstream in{prefix_ + "/" + std::to_string(lbl_id + 1) + ".bin",
                         std::ios::binary};

        // read the label first
        std::string lbl;
        io::packed::read(in, lbl);

        for (uint64_t i = 0; i < features_per_class; ++i)
        {
            io::packed::read(in, tid);
            io::packed::read(in, score);

            positions.push_back(tid);

            prog((lbl_id + 1) * (i + 1));
        }

        prog.end();
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()),
                    positions.end());

    // ensure destruction of old sarray and rank/select structures
    s_rank_ = util::nullopt;
    s_select_ = util::nullopt;
    sarray_ = util::nullopt;
    filesystem::remove_all(prefix_ + "/sarray");

    // hallucinate an additional feature to fix boundary cases later
    sarray_ = succinct::make_sarray(prefix_ + "/sarray", positions.begin(),
                                    positions.end(), total_features_ + 1);
    s_rank_ = succinct::sarray_rank{prefix_ + "/sarray", *sarray_};
    s_select_ = succinct::sarray_select{prefix_ + "/sarray", *sarray_};
}

bool feature_selector::selected(term_id tid) const
{
    // rank(tid) returns the number of ones in the "bit vector" of selected
    // features that occur *before* the tid-th position.
    //
    // If rank(tid) < rank(tid + 1), then it must be the case that the
    // tid-th bit was set to one since the rank incremented. If they are
    // equal, the tid-th bit could not have been set (since there isn't one
    // more one before the tid+1-th bit.
    return s_rank_->rank(tid) < s_rank_->rank(tid + 1);
}

learn::feature_id feature_selector::new_id(term_id term) const
{
    // rank(tid) returns the number of ones in the "bit vector" of selected
    // features that occur *before* the tid-th position.
    //
    // This provides a condensed re-labeling for the term ids starting at 0
    // and incremented for each new selected feature.
    assert(selected(term));
    return learn::feature_id{s_rank_->rank(term)};
}

term_id feature_selector::old_id(learn::feature_id feature) const
{
    // select(feature) returns the position of the feature-th one in the
    // "bit vector" of selected features
    return term_id{s_select_->select(feature)};
}

uint64_t feature_selector::total_selected() const
{
    return s_rank_->size();
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
        std::ifstream in{prefix_ + "/" + std::to_string(lbl_id + 1) + ".bin",
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
