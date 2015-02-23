/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <cassert>
#include <unordered_set>
#include "cpptoml.h"
#include "classify/classifier/naive_bayes.h"
#include "index/postings_data.h"

namespace meta
{
namespace classify
{

const std::string naive_bayes::id = "naive-bayes";

naive_bayes::naive_bayes(std::shared_ptr<index::forward_index> idx,
                         double alpha, double beta)
    : classifier{std::move(idx)},
      class_probs_{stats::dirichlet<class_label>{beta, idx_->num_labels()}}
{
    stats::dirichlet<term_id> term_prior{alpha, idx_->unique_terms()};
    auto lbls = idx_->class_labels();
    std::sort(std::begin(lbls), std::end(lbls));
    term_probs_.reserve(lbls.size());
    for (const auto& lbl : lbls)
        term_probs_.emplace_back(lbl, term_prior);
}

void naive_bayes::reset()
{
    for (auto& term_dist : term_probs_)
        term_dist.second.clear();
    class_probs_.clear();
}

void naive_bayes::train(const std::vector<doc_id>& docs)
{
    for (auto& d_id : docs)
    {
        auto pdata = idx_->search_primary(d_id);
        auto lbl = idx_->label(d_id);
        for (auto& p : pdata->counts())
        {
            term_probs_[lbl].increment(p.first, p.second);
            assert(term_probs_[lbl].probability(p.first) > 0);
        }
        class_probs_.increment(lbl, 1);
    }
}

class_label naive_bayes::classify(doc_id d_id)
{
    class_label label;
    double best = std::numeric_limits<double>::lowest();

    // calculate prob of test doc for each class
    for (auto& cls : term_probs_)
    {
        const auto& lbl = cls.first;
        const auto& term_dist = cls.second;

        double sum = 0.0;
        assert(class_probs_.probability(lbl) > 0);
        sum += log(class_probs_.probability(lbl));
        auto pdata = idx_->search_primary(d_id);
        for (auto& t : pdata->counts())
        {
            assert(term_dist.probability(t.first) > 0);
            sum += t.second * log(term_dist.probability(t.first));
        }

        if (sum > best)
        {
            best = sum;
            label = cls.first;
        }
    }

    return label;
}

template <>
std::unique_ptr<classifier>
    make_classifier<naive_bayes>(const cpptoml::table& config,
                                 std::shared_ptr<index::forward_index> idx)
{
    auto alpha = naive_bayes::default_alpha;
    if (auto c_alpha = config.get_as<double>("alpha"))
        alpha = *c_alpha;

    auto beta = naive_bayes::default_beta;
    if (auto c_beta = config.get_as<double>("beta"))
        beta = *c_beta;

    return make_unique<naive_bayes>(std::move(idx), alpha, beta);
}
}
}
