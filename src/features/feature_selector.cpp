/**
 * @file feature_selector.cpp
 * @author Sean Massung 
 * @author Siddharth Shukramani
 */

#include <fstream>
#include <iostream>
#include "meta/features/feature_selector.h"
#include "meta/io/packed.h"
#include "meta/parallel/parallel_for.h"

namespace meta
{
namespace features
{
feature_selector::feature_selector(const std::string& prefix,
                                   uint64_t total_features)
    : prefix_{prefix},
      selected_{prefix_ + ".selected", total_features}
{
    // nothing
}

void feature_selector::score_all()
{
    using pair_t = std::pair<term_id, double>;
    using pair_c = std::pair<class_label, std::vector<pair_t>>;

    printing::progress prog{" > Selecting features: ", term_prob_.unique_events()};

    std::vector<pair_c> scores;

    uint64_t num_processed = 0;

    class_prob_.each_seen_event([&](const class_label& lbl)
    {
        std::vector<pair_t> class_scores;

        term_prob_.each_seen_event([&](const term_id& tid)
        {
            prog(++num_processed);

            class_scores.push_back(std::make_pair(tid, score(lbl, tid)));
        });

        scores.push_back(std::make_pair(lbl, class_scores));
    });

    prog.end();

    parallel::parallel_for(
        scores.begin(), scores.end(), [&](pair_c & c_scores)
        {
            std::sort(c_scores.second.begin(), c_scores.second.end(),
                      [](const pair_t& a, const pair_t& b)
                      {
                          return a.second > b.second;
                      });

            std::ofstream out{prefix_ + "." + 
                              static_cast<std::string>(c_scores.first),
                              std::ios::binary};

            for (auto& score : c_scores.second)
            {
                io::packed::write(out, score.first);
                io::packed::write(out, score.second);
            }
        });
}

void feature_selector::select(uint64_t features_per_class /* = 20 */)
{
    // zero out old vector
    for (auto& b : selected_)
        b = false;

    term_id tid;
    double score;
	
    class_prob_.each_seen_event([&](const class_label& lbl)
    {
        std::ifstream in{prefix_ + "." +
                         static_cast<std::string>(lbl),
                         std::ios::binary};

        for (uint64_t i = 0; i < features_per_class; ++i)
        {	
            io::packed::read(in, tid);
            io::packed::read(in, score);
            selected_[tid] = true;
        }
    });
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

    double num_features = p * term_prob_.unique_events();
    // truncate to int
    auto per_class = static_cast<uint64_t>(num_features/class_prob_.unique_events());
	
    select(per_class);
}

void feature_selector::print_summary(std::shared_ptr<index::disk_index> idx, 
									 uint64_t k /* = 20 */) const
{
    term_id tid;
    double score;	

    class_prob_.each_seen_event([&](const class_label& lbl)
    {
        std::cout << std::endl
                  << "Top " << k << " features for \""
                  << lbl << "\":" << std::endl
                  << "===============================" << std::endl;

        // read (term_id, score) pairs
        std::ifstream in{prefix_ + "." + static_cast<std::string>(lbl),
                         std::ios::binary};

        for (uint64_t i = 0; i < k; ++i)
        {
            io::packed::read(in, tid);
            io::packed::read(in, score); 
            std::cout << (i + 1) << ". " << idx->term_text(tid) << " ("
                      << score << ")" << std::endl;
        }
    });
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

double feature_selector::prob_class(class_label lbl) const
{
    auto p = class_prob_.probability(lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::term_and_class(term_id tid, class_label lbl) const
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
                                                class_label lbl) const
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

double feature_selector::term_and_not_class(term_id tid, class_label lbl) const
{
    auto p = term_prob_.probability(tid) - term_and_class(tid, lbl);
#if DEBUG
    if (p < 0 || p > 1)
        throw std::runtime_error{std::string{__func__} + ": "
                                 + std::to_string(p)};
#endif
    return p;
}

double feature_selector::not_term_and_class(term_id tid, class_label lbl) const
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
