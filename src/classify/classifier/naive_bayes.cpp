/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <unordered_set>
#include "cluster/similarity.h"
#include "classify/classifier/naive_bayes.h"
#include "index/postings_data.h"

namespace meta
{
namespace classify
{

const std::string naive_bayes::id = "naive-bayes";

naive_bayes::naive_bayes(std::shared_ptr<index::forward_index> idx,
                         double alpha, double beta)
    : classifier{std::move(idx)}, total_docs_{0}, alpha_{alpha}, beta_{beta}
{
    /* nothing */
}

void naive_bayes::reset()
{
    term_probs_.clear();
    class_counts_.clear();
    total_docs_ = 0;
}

void naive_bayes::train(const std::vector<doc_id>& docs)
{
    for (auto& d_id : docs)
    {
        ++total_docs_;
        auto pdata = idx_->search_primary(d_id);
        for (auto& p : pdata->counts())
            term_probs_[idx_->label(d_id)][p.first] += p.second;
        ++class_counts_[idx_->label(d_id)];
    }

    // calculate P(term|class) for all classes based on c(term|class)
    for (auto& cls : term_probs_)
    {
        for (auto& p : cls.second)
            p.second /= class_counts_[cls.first];
    }
}

class_label naive_bayes::classify(doc_id d_id)
{
    class_label label;
    double best = std::numeric_limits<double>::min();

    // calculate prob of test doc for each class
    for (auto& cls : term_probs_)
    {
        double sum = 0.0;
        double class_prob = static_cast<double>(class_counts_.at(cls.first))
                            / total_docs_;
        class_prob += beta_;
        sum += log(1 + class_prob);
        auto pdata = idx_->search_primary(d_id);
        for (auto& t : pdata->counts())
        {
            auto it = cls.second.find(t.first);
            double term_prob = (it == cls.second.end()) ? 0.0 : it->second;
            term_prob += alpha_;
            sum += log(1 + term_prob);
        }

        if (sum > best)
        {
            best = sum;
            label = cls.first;
        }
    }

    return label;
}
}
}
