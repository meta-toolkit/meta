/**
 * @file naive_bayes.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <unordered_set>
#include "cluster/similarity.h"
#include "classify/naive_bayes.h"

namespace meta {
namespace classify {

using std::vector;
using std::cerr;
using std::endl;
using std::unordered_map;
using std::unordered_set;
using index::Document;

naive_bayes::naive_bayes(double alpha, double beta):
    _term_probs(unordered_map<ClassLabel, unordered_map<TermID, double>>()),
    _class_counts(unordered_map<ClassLabel, size_t>()),
    _total_docs(0),
    _alpha(alpha),
    _beta(beta)
{ /* nothing */ }

void naive_bayes::train(const vector<Document> & docs)
{
    using namespace clustering::similarity::internal;

    // discover all seen features
    cerr << "  calculating feature space..." << endl;
    unordered_set<TermID> term_space;
    for(auto & d: docs)
    {
        ++_total_docs;
        for(auto & p: d.getFrequencies())
            term_space.insert(p.first);
    }

    // calculate c(term|class) for all classes
    cerr << "  calculating term probabilities..." << endl;
    for(auto & t: term_space)
    {
        for(auto & d: docs)
        {
            ++_class_counts[d.getCategory()];
            size_t count = common::safe_at(d.getFrequencies(), t);
            _term_probs[d.getCategory()][t] += count;
        }
    }

    // calculate P(term|class) for all classes based on c(term|class)
    for(auto & cls: _term_probs)
    {
        for(auto & p: cls.second)
            p.second /= _class_counts[cls.first];
    }
   
}

ClassLabel naive_bayes::classify(const Document & doc) const
{
    ClassLabel label;
    double best = std::numeric_limits<double>::min();

    // calculate prob of test doc for each class
    for(auto & cls: _term_probs)
    {
        double sum = 0.0;
        double class_prob = static_cast<double>(_class_counts.at(cls.first)) / _total_docs;
        class_prob += _beta;
        for(auto & t: doc.getFrequencies())
        {
            auto it = cls.second.find(t.first);
            double term_prob = (it == cls.second.end()) ? 0.0 : it->second;
            term_prob += _alpha;
            sum += log(1 + term_prob * class_prob);
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
