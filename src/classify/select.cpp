/**
 * @file select.cpp
 */

#include "classify/select.h"

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::pair;
using std::string;

unordered_map<string, vector<Document>> classify::feature_select::partition_classes(
        const vector<Document> & docs)
{
    unordered_map<string, vector<Document>> classes;
    for(auto & d: docs)
        classes[d.getCategory()].push_back(d);
    return classes;
}

unordered_set<string> classify::feature_select::get_class_space(const vector<Document> & docs)
{
    unordered_set<string> classes;
    for(auto & d: docs)
        classes.insert(d.getCategory());
    return classes;
}

unordered_set<TermID> classify::feature_select::get_term_space(const vector<Document> & docs)
{
    unordered_set<TermID> terms;
    for(auto & d: docs)
        for(auto & p: d.getFrequencies())
            terms.insert(p.first);
    return terms;
}

double classify::feature_select::term_given_class(TermID term, const string & label,
        unordered_map<string, vector<Document>> classes)
{
    size_t count = 0;
    for(auto & doc: classes[label])
        if(doc.getFrequencies().find(term) != doc.getFrequencies().end())
            ++count;
    return static_cast<double>(count) / classes[label].size();
}


double classify::feature_select::not_term_given_not_class(TermID term, const string & label,
        unordered_map<string, vector<Document>> classes)
{
    size_t count = 0;
    size_t total = 0;
    for(auto & cls: classes)
    {
        if(cls.first != label)
        {
            for(auto & doc: cls.second)
            {
                ++total;
                if(doc.getFrequencies().find(term) == doc.getFrequencies().end())
                    ++count;
            }
        }
    }
    return static_cast<double>(count) / total;
}
