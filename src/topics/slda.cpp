/**
 * @file slda.cpp
 * @author Sean Massung
 */

#include "topics/slda.h"

namespace meta {
namespace topics {

using std::vector;
using std::unordered_map;
using std::pair;
using index::Document;

slda::slda(double alpha):
    _alpha(alpha)
{ /* nothing */ }

void slda::estimate(const vector<Document> & docs)
{

}

unordered_map<class_label, vector<pair<term_id, double>>> slda::class_distributions() const
{
    unordered_map<class_label, vector<pair<term_id, double>>> ret;
    return ret;
}

vector<pair<term_id, double>> slda::select_features() const
{
    vector<pair<term_id, double>> ret;
    return ret;
}

void slda::infer(const vector<Document> & docs) const
{

}

}
}
