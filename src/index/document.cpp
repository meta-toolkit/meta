/**
 * @file document.cpp
 * @author Sean Massung
 */

#include <utility>
#include <sstream>
#include "index/document.h" 
#include "cluster/similarity.h"

namespace meta {
namespace index {

using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using util::InvertibleMap;

#include <iostream>
using namespace std;

document::document(const string & path, const class_label & label):
    _path(path),
    _label(label),
    _length(0),
    _frequencies(unordered_map<term_id, unsigned int>())
{

    size_t idx = path.find_last_of("/") + 1;
    _name = path.substr(idx);
}

void document::increment(term_id termID, unsigned int amount,
        std::shared_ptr<unordered_map<term_id, unsigned int>> docFreq)
{
    auto iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        iter->second += amount;
    else
    {
        _frequencies.insert(make_pair(termID, amount));
        if(docFreq)
            (*docFreq)[termID]++;
    }

    _length += amount;
}

void document::increment(term_id termID, unsigned int amount)
{
    increment(termID, amount, nullptr);
}

string document::path() const
{
    return _path;
}

string document::label() const
{
    return _label;
}

string document::name() const
{
    return _name;
}

size_t document::length() const
{
    return _length;
}

size_t document::frequency(term_id termID) const
{
    auto iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        return iter->second;
    else
        return 0;
}

const unordered_map<term_id, unsigned int> & document::frequencies() const
{
    return _frequencies;
}

string document::get_liblinear_data(InvertibleMap<string, int> & mapping) const
{
    stringstream out;
    out << get_mapping(mapping, _label);     // liblinear, classes start at 1
    vector<pair<term_id, unsigned int>> sorted;

    // liblinear feature indices start at 1, not 0 like the tokenizers
    for(auto & freq: _frequencies)
        sorted.push_back(make_pair(freq.first + 1, freq.second));

    std::sort(sorted.begin(), sorted.end(),
        [](const pair<term_id, unsigned int> & a, const pair<term_id, unsigned int> & b) {
            return a.first < b.first;
        }
    );

    for(auto & freq: sorted)
        out << " " << freq.first << ":" << freq.second;

    out << "\n";
    return out.str();
}

int document::get_mapping(InvertibleMap<string, int> & mapping, const class_label & category)
{
    // see if entered already
    if(mapping.containsKey(category))
        return mapping.getValueByKey(category);

    // otherwise, inefficiently create new entry (starting at 1)
    int newVal = 1;
    while(mapping.containsValue(newVal))
        ++newVal;

    mapping.insert(category, newVal);
    return newVal;
}

double document::cosine_similarity(const document & a, const document & b)
{
    return clustering::similarity::cosine_similarity(a._frequencies, b._frequencies);
}

double document::jaccard_similarity(const document & a, const document & b)
{
    return clustering::similarity::jaccard_similarity(a._frequencies, b._frequencies);
}

vector<document> document::load_docs(const string & filename, const string & prefix)
{
    vector<document> docs;
    std::ifstream infile{filename};
    string line;
    while(infile.good())
    {
        std::getline(infile, line);
        if(line == "")
            continue;
        string file = line;
        size_t space = line.find_first_of(" ");
        // see if there is class label info for this doc
        if(space != string::npos)
        {
            class_label label = line.substr(0, space);
            file = line.substr(space + 1);
            docs.emplace_back(document{prefix + "/" + file, label});
        }
        else
            docs.emplace_back(document{prefix + "/" + file});
    }
    infile.close();
    return docs;
}

string document::get_slda_term_data() const
{
    stringstream out;
    out << _frequencies.size();
    for(auto & f: _frequencies)
        out << " " << f.first << ":" << f.second;
    out << "\n";
    return out.str();
}

string document::get_slda_label_data(InvertibleMap<class_label, int> & mapping) const
{
    // minus one because slda classes start at 0
    return common::to_string(get_mapping(mapping, _label) - 1) + "\n";
}

document document::filter_features(const document & doc,
        const vector<pair<term_id, double>> & features)
{
    document filtered(doc);
    for(auto & feature: features)
    {
        auto it = filtered._frequencies.find(feature.first);
        if(it != filtered._frequencies.end())
            filtered._frequencies.erase(it);
    }
    return filtered;
}

vector<document> document::filter_features(const vector<document> & docs,
        const vector<pair<term_id, double>> & features)
{
    vector<document> ret;
    ret.reserve(docs.size());
    for(auto & doc: docs)
        ret.emplace_back(filter_features(doc, features));
    return ret;
}

}
}
