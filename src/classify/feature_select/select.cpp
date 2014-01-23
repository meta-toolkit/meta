/**
 * @file select.cpp
 */

#include <string>
#include "cluster/similarity.h"
#include "parallel/parallel_for.h"
#include "util/common.h"
#include "classify/feature_select/select.h"

namespace meta {
namespace classify {

using std::unordered_map;
using std::unordered_set;
using std::pair;

feature_select::feature_select(const std::vector<corpus::document> & docs):
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

void feature_select::set_pseen(const std::vector<corpus::document> & docs)
{
#if 0
    // set all probs to zero initially; this makes sure they are entered in the
    // map, avoiding exceptions when using at()
    for(auto & t: _term_space)
    {
        for(auto & c: _class_space)
        {
            _pseen[c][t] = 0;
            _pterm[t] = 0;
        }
    }

    // aggregate counts
    for(auto & d: docs)
    {
        for(auto & f: d.counts())
        {
            size_t count = f.second;
            _pseen[d.label()][f.first] += count;
            _pterm[f.first] += count;
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
#endif
}

std::vector<pair<term_id, double>>
feature_select::sort_terms(unordered_map<term_id, double> & weights) const
{
    std::vector<pair<term_id, double>> ret(weights.begin(), weights.end());
    std::sort(ret.begin(), ret.end(),
        [](const pair<term_id, double> & a, const pair<term_id, double> & b) {
            return a.second > b.second;
        }
    );
    return ret;
}

void feature_select::set_class_space(const std::vector<corpus::document> & docs)
{
#if 0
    for(auto & d: docs)
    {
        ++_pclass[d.label()];
        _class_space.insert(d.label());
    }

    for(auto & c: _pclass)
        c.second /= docs.size();
#endif
}

void feature_select::set_term_space(const std::vector<corpus::document> & docs)
{
#if 0
    for(auto & d: docs)
    {
        _num_terms += d.length();
        for(auto & p: d.counts())
            _term_space.insert(p.first);
    }
#endif
}

double feature_select::term_and_class(term_id term, const class_label & label) const
{
    return _pseen.at(label).at(term);
}


double feature_select::not_term_and_not_class(term_id term, const class_label & label) const
{
    double prob = 1.0 - term_and_class(term, label)
               - not_term_and_class(term, label)
               - term_and_not_class(term, label);
    return prob;
}

double feature_select::term_and_not_class(term_id term, const class_label & label) const
{
    return _pterm.at(term) - term_and_class(term, label);
}

double feature_select::not_term_and_class(term_id term, const class_label & label) const
{
    return _pclass.at(label) - term_and_class(term, label);
}

}
}
