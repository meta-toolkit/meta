/**
 * @file feature_selector.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <fstream>
#include "util/filesystem.h"
#include "util/progress.h"
#include "parallel/parallel_for.h"
#include "features/feature_selector.h"
#include "index/postings_data.h"
#include "io/binary.h"

namespace meta
{
namespace features
{
feature_selector::feature_selector(const std::string& prefix,
                                   std::shared_ptr<index::forward_index> idx)
    : prefix_{prefix},
      idx_{std::move(idx)},
      selected_{prefix_ + ".selected", idx_->unique_terms()}
{ /* nothing */
}

void feature_selector::init()
{
    // if the first class distribution doesn't exist, we haven't created the
    // data for this feature_selector yet
    if (!filesystem::file_exists(prefix_ + ".0"))
    {
        // initially set all probabilities to zero; this allows fast random
        // access to the probabilities
        term_prob_.assign(idx_->unique_terms(), 0.0);
        class_prob_.assign(idx_->num_labels(), 0.0);
        co_occur_.assign(idx_->num_labels(),
                         std::vector<double>(idx_->unique_terms(), 0.0));
        calc_probs();
        score_all();
    }
    else
    {
        std::cout << "Loading feature_selector not implemented yet"
                  << std::endl;
    }
}

void feature_selector::score_all()
{
    using pair_t = std::pair<term_id, double>;
    std::vector<std::vector<pair_t>> scores(
        class_prob_.size(), std::vector<pair_t>(term_prob_.size()));

    printing::progress prog{" > Selecting features: ", term_prob_.size()};
    for (uint64_t tid = 0; tid < term_prob_.size(); ++tid)
    {
        prog(tid);
        for (uint64_t lbl = 0; lbl < class_prob_.size(); ++lbl)
            scores[lbl][tid]
                = std::make_pair(tid, score(label_id{lbl + 1}, term_id{tid}));
    }
    prog.end();

    parallel::parallel_for(scores.begin(), scores.end(), [&](auto& v)
                           {
        std::sort(v.begin(), v.end(), [&](const auto& a, const auto& b)
                  {
            return a.second < b.second;
        });
    });

    for (uint64_t lbl = 0; lbl < class_prob_.size(); ++lbl)
    {
        // write (term_id, score) pairs
        std::ofstream out{prefix_ + "." + std::to_string(lbl + 1)};
        for (auto& score : scores[lbl])
        {
            io::write_binary(out, score.first);
            io::write_binary(out, score.second);
        }
    }
}

void feature_selector::select(uint64_t k /* = 25 */)
{
}

bool feature_selector::selected(term_id term) const
{
    return false;
}

void feature_selector::select_percent(double p /* = 0.05 */)
{
    double num_features = p * idx_->unique_terms();
    uint64_t per_class = num_features / idx_->num_labels(); // truncate to int
    select(per_class);
}

void feature_selector::calc_probs()
{
    printing::progress prog{" > Calculating feature probs: ", idx_->num_docs()};
    uint64_t total_terms = 0;
    for (doc_id did = doc_id{0}; did < idx_->num_docs(); ++did)
    {
        prog(did);
        auto lid = idx_->lbl_id(did);
        ++class_prob_[lid - 1];
        for (auto& count : idx_->search_primary(did)->counts())
        {
            term_prob_[count.first] += count.second;
            co_occur_[lid - 1][count.first] += count.second;
            total_terms += count.second;
        }
    }
    prog.end();

    for (auto& p : class_prob_)
        p /= idx_->num_labels();

    for (auto& p : term_prob_)
        p /= total_terms;

    for (auto& probs : co_occur_)
        for (auto& p : probs)
            p /= total_terms;
}

void feature_selector::print_summary(uint64_t k /* = 20 */) const
{
    term_id tid;
    double score;
    for (uint64_t lbl = 0; lbl < class_prob_.size(); ++lbl)
    {
        std::cout << std::endl << "Top " << k << " features for \""
                  << idx_->class_label_from_id(label_id{lbl + 1})
                  << "\":" << std::endl
                  << "===============================" << std::endl;

        // read (term_id, score) pairs
        std::ifstream in{prefix_ + "." + std::to_string(lbl + 1)};
        for (uint64_t i = 0; i < k; ++i)
        {
            io::read_binary(in, tid);
            io::read_binary(in, score);
            std::cout << (i + 1) << ". " << idx_->term_text(tid) << " ("
                      << score << ")" << std::endl;
        }
    }
}

double feature_selector::prob_term(term_id id) const
{
    return term_prob_.at(id);
}

double feature_selector::prob_class(label_id id) const
{
    return class_prob_.at(id - 1);
}

double feature_selector::term_and_class(term_id term, label_id label) const
{
    return co_occur_.at(label - 1).at(term);
}

double feature_selector::not_term_and_not_class(term_id term,
                                                label_id label) const
{
    return 1.0 - term_and_class(term, label) - not_term_and_class(term, label)
           - term_and_not_class(term, label);
}

double feature_selector::term_and_not_class(term_id term, label_id label) const
{
    return term_prob_.at(term) - term_and_class(term, label);
}

double feature_selector::not_term_and_class(term_id term, label_id label) const
{
    return class_prob_.at(label - 1) - term_and_class(term, label);
}
}
}
