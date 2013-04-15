/**
 * @file slda.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <unordered_set>
#include "topics/slda.h"

namespace meta {
namespace topics {

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::string;
using util::InvertibleMap;
using index::Document;

slda::slda(const string & slda_path, double alpha):
    _alpha(alpha),
    _slda_path(slda_path),
    _mapping(InvertibleMap<class_label, int>())
{ /* nothing */ }

void slda::estimate(const vector<Document> & docs)
{
    std::ofstream data("slda-data");
    std::ofstream labels("slda-labels");

    unordered_set<class_label> unique_labels;

    for(auto & d: docs)
    {
        labels << d.get_slda_label_data(_mapping);
        data << d.get_slda_term_data();
        unique_labels.insert(d.getCategory());
    }

    labels.close();
    data.close();

    string command = _slda_path + "/slda est slda-data slda-labels ";
    command += _slda_path + "/settings.txt "; // use default settings for now
    command += common::to_string(_alpha) + " ";
    command += common::to_string(unique_labels.size()) + " ";
    command += "random slda-est-output";
    system(command.c_str());
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
