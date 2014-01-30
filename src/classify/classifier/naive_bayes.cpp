/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <unordered_set>
#include "cluster/similarity.h"
#include "classify/classifier/naive_bayes.h"
#include "index/postings_data.h"

namespace meta {
namespace classify {

naive_bayes::naive_bayes(index::forward_index & idx,
                         double alpha, double beta):
    classifier{idx},
    _total_docs{0},
    _alpha{alpha},
    _beta{beta}
{ /* nothing */ }

void naive_bayes::reset()
{
    _term_probs.clear();
    _class_counts.clear();
    _total_docs = 0;
}

void naive_bayes::train(const std::vector<doc_id> & docs)
{
    for(auto & d_id: docs)
    {
        ++_total_docs;
        auto pdata = _idx.search_primary(d_id);
        for(auto & p: pdata->counts())
            _term_probs[_idx.label(d_id)][p.first] += p.second;
        ++_class_counts[_idx.label(d_id)];
    }

    // calculate P(term|class) for all classes based on c(term|class)
    for(auto & cls: _term_probs)
    {
        for(auto & p: cls.second)
            p.second /= _class_counts[cls.first];
    }
}

class_label naive_bayes::classify(doc_id d_id)
{
    class_label label;
    double best = std::numeric_limits<double>::min();

    // calculate prob of test doc for each class
    for(auto & cls: _term_probs)
    {
        double sum = 0.0;
        double class_prob =
            static_cast<double>(_class_counts.at(cls.first)) / _total_docs;
        class_prob += _beta;
        sum += log(1 + class_prob);
        auto pdata = _idx.search_primary(d_id);
        for(auto & t: pdata->counts())
        {
            auto it = cls.second.find(t.first);
            double term_prob = (it == cls.second.end()) ? 0.0 : it->second;
            term_prob += _alpha;
            sum += log(1 + term_prob);
        }

        if(sum > best)
        {
            best = sum;
            label = cls.first;
        }
    }

    return label;
}

}
}
