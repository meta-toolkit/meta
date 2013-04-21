/**
 * @file document.cpp
 * @author Sean Massung
 */

#include <map>
#include <utility>
#include <iostream>
#include <sstream>
#include "index/document.h" 
#include "cluster/similarity.h"

namespace meta {
namespace index {

using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::pair;
using std::make_pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;

using util::InvertibleMap;

Document::Document(const string & path):
    _path(path),
    _length(0),
    _frequencies(unordered_map<term_id, unsigned int>())
{
    _name = getName(path);
    _category = getCategory(path);
}

void Document::increment(term_id termID, unsigned int amount,
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

void Document::increment(term_id termID, unsigned int amount)
{
    increment(termID, amount, nullptr);
}

string Document::getPath() const
{
    return _path;
}

string Document::getCategory() const
{
    return _category;
}

string Document::getName() const
{
    return _name;
}

size_t Document::getLength() const
{
    return _length;
}

size_t Document::getFrequency(term_id termID) const
{
    auto iter = _frequencies.find(termID);
    if(iter != _frequencies.end())
        return iter->second;
    else
        return 0;
}

const unordered_map<term_id, unsigned int> & Document::getFrequencies() const
{
    return _frequencies;
}

string Document::getName(const string & path)
{
    size_t idx = path.find_last_of("/") + 1;
    return path.substr(idx, path.size() - idx);
}

string Document::getCategory(const string & path)
{
    size_t idx = path.find_last_of("/");
    string sub = path.substr(0, idx);
    return getName(sub);
}

string Document::get_liblinear_data(InvertibleMap<string, int> & mapping) const
{
    stringstream out;
    out << getMapping(mapping, getCategory());     // liblinear, classes start at 1
    map<term_id, unsigned int> sorted;

    // liblinear feature indices start at 1, not 0 like the tokenizers
    for(auto & freq: _frequencies)
        sorted.insert(make_pair(freq.first + 1, freq.second));

    for(auto & freq: sorted)
        out << " " << freq.first << ":" << freq.second;

    out << "\n";
    return out.str();
}

int Document::getMapping(InvertibleMap<string, int> & mapping, const string & category)
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

double Document::cosine_similarity(const Document & a, const Document & b)
{
    return clustering::similarity::cosine_similarity(a._frequencies, b._frequencies);
}

double Document::jaccard_similarity(const Document & a, const Document & b)
{
    return clustering::similarity::jaccard_similarity(a._frequencies, b._frequencies);
}

vector<Document> Document::loadDocs(const string & filename, const string & prefix)
{
    vector<Document> docs;
    io::Parser parser(filename, "\n");
    while(parser.hasNext())
    {
        string file = parser.next();
        docs.push_back(Document(prefix + "/" + file));
    }
    return docs;
}

string Document::get_slda_term_data() const
{
    stringstream out;
    out << _frequencies.size();
    for(auto & f: _frequencies)
        out << " " << f.first << ":" << f.second;
    out << "\n";
    return out.str();
}

string Document::get_slda_label_data(InvertibleMap<class_label, int> & mapping) const
{
    // minus one because slda classes start at 0
    return common::to_string(getMapping(mapping, getCategory()) - 1) + "\n";
}

Document Document::filter_features(const Document & doc,
        const vector<pair<term_id, double>> & features)
{
    Document filtered(doc);
    for(auto & feature: features)
    {
        auto it = filtered._frequencies.find(feature.first);
        if(it != filtered._frequencies.end())
            filtered._frequencies.erase(it);
    }
    return filtered;
}

vector<Document> Document::filter_features(const vector<Document> & docs,
        const vector<pair<term_id, double>> & features)
{
    vector<Document> ret;
    ret.reserve(docs.size());
    for(auto & doc: docs)
        ret.emplace_back(filter_features(doc, features));
    return ret;
}

}
}
