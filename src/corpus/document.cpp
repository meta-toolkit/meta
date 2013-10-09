/**
 * @file document.cpp
 * @author Sean Massung
 */

#include <utility>
#include <sstream>
#include "corpus/document.h"
#include "corpus/corpus.h"
#include "util/common.h"
#include "parallel/parallel_for.h"
#include "cluster/similarity.h"

namespace meta {
namespace corpus {

using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using util::invertible_map;

document::document(const string & path, doc_id d_id, const class_label & label):
    _path{path},
    _d_id{d_id},
    _label{label},
    _length{0},
    _content{""},
    _contains_content{false}
{

    size_t idx = path.find_last_of("/") + 1;
    _name = path.substr(idx);
}

void document::increment(term_id termID, double amount)
{
    _frequencies[termID] += amount;
    _length += amount;
}

string document::path() const
{
    return _path;
}

class_label document::label() const
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

double document::frequency(term_id termID) const
{
    return common::safe_at(_frequencies, termID);
}

const unordered_map<term_id, double> & document::frequencies() const
{
    return _frequencies;
}

double document::cosine_similarity(const document & a, const document & b)
{
    return clustering::similarity::cosine_similarity(a._frequencies,
                                                     b._frequencies);
}

double document::jaccard_similarity(const document & a, const document & b)
{
    return clustering::similarity::jaccard_similarity(a._frequencies,
                                                      b._frequencies);
}

document document::filter_features(const document & doc,
        const vector<pair<term_id, double>> & features)
{
    std::unordered_set<term_id> keep;
    keep.reserve(features.size());
    for(auto & p: features)
        keep.insert(p.first);

    document filtered(doc._path, doc._d_id, doc._label);
    
    filtered._frequencies.clear();
    for(auto & f: doc._frequencies)
    {
        auto it = keep.find(f.first);
        if(it != keep.end())
        {
            filtered._frequencies[f.first] = f.second;
            filtered._length += f.second;
        }
    } 

    return filtered;
}

vector<document> document::filter_features(const vector<document> & docs,
        const vector<pair<term_id, double>> & features)
{
    vector<document> ret;
    ret.reserve(docs.size());
    std::mutex _mutex;
    parallel::parallel_for(docs.begin(), docs.end(), [&](const document & doc)
    {
        document filtered{filter_features(doc, features)};
        {
            std::lock_guard<std::mutex> lock{_mutex};
            ret.emplace_back(filtered);
        }
    });
    return ret;
}

void document::set_content(const std::string & content)
{
    _content = content;
    _contains_content = true;
}

const std::string & document::content() const
{
    if(_contains_content)
        return _content;
    throw corpus::corpus_exception{
        "there is no content for the requested document"};
}

doc_id document::id() const
{
    return _d_id;
}

bool document::contains_content() const
{
    return _contains_content;
}

}
}
