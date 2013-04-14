/**
 * @file select.cpp
 */

#include <cassert> // debug
#include "cluster/similarity.h"
#include "util/common.h"
#include "classify/select.h"

namespace meta {
namespace classify {

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::pair;
using index::Document;

feature_select::feature_select(const vector<Document> & docs):
    _term_space(unordered_set<term_id>()),
    _class_space(unordered_set<class_label>()),
    _num_terms(0),
    _pterm(unordered_map<term_id, double>()),
    _pclass(unordered_map<class_label, double>())
{
    set_term_space(docs);
    set_class_space(docs);
    set_pseen(docs);
}

void feature_select::set_pseen(const vector<Document> & docs)
{
    // aggregate counts
    for(auto & t: _term_space)
    {
        for(auto & d: docs)
        {
            size_t count = common::safe_at(d.getFrequencies(), t);
            _pseen[d.getCategory()][t] += count;
            _pterm[t] += count;
        }
    }

    // create probabilities

    for(auto & t: _pterm)
        t.second /= _num_terms;

    for(auto & c: _pseen)
    {
        for(auto & p: c.second)
            p.second /= _num_terms;
    }
}

vector<pair<term_id, double>> feature_select::sort_terms(unordered_map<term_id, double> & weights) const
{
    vector<pair<term_id, double>> ret(weights.begin(), weights.end());
    std::sort(ret.begin(), ret.end(),
        [](const pair<term_id, double> & a, const pair<term_id, double> & b) {
            return a.second > b.second;
        }
    );
    return ret;
}

void feature_select::set_class_space(const vector<Document> & docs)
{
    for(auto & d: docs)
    {
        ++_pclass[d.getCategory()];
        _class_space.insert(d.getCategory());
    }

    for(auto & c: _pclass)
        c.second /= docs.size();
}

void feature_select::set_term_space(const vector<Document> & docs)
{
    for(auto & d: docs)
    {
        _num_terms += d.getLength();
        for(auto & p: d.getFrequencies())
            _term_space.insert(p.first);
    }
}

double feature_select::term_and_class(term_id term, const class_label & label) const
{
    double prob = _pseen.at(label).at(term);
    assert(prob <= 1.0 && prob >= 0.0); // debug
    return prob;
}


double feature_select::not_term_and_not_class(term_id term, const class_label & label) const
{
    double prob = 1.0 - term_and_class(term, label)
               - not_term_and_class(term, label)
               - term_and_not_class(term, label);
    assert(prob <= 1.0 && prob >= 0.0);
    return prob;
}

double feature_select::term_and_not_class(term_id term, const class_label & label) const
{
    double prob = _pterm.at(term) - term_and_class(term, label);
    assert(prob <= 1.0 && prob >= 0.0);
    return prob;
}

double feature_select::not_term_and_class(term_id term, const class_label & label) const
{
    double prob = _pclass.at(label) - term_and_class(term, label);
    assert(prob <= 1.0 && prob >= 0.0);
    return prob;
}

}
}
