/**
 * @file select_slda.cpp
 */

#include <unordered_map>
#include <utility>
#include "topics/slda.h"
#include "classify/select_slda.h"

namespace meta {
namespace classify {

using std::unordered_map;
using std::pair;
using std::vector;
using index::Document;

select_slda::select_slda(const vector<Document> & docs):
    _docs(docs) { /* nothing */ }

vector<pair<term_id, double>> select_slda::select()
{
    vector<pair<term_id, double>> features;
    return features;
}

std::unordered_map<class_label, std::vector<std::pair<term_id, double>>>
select_slda::select_by_class() {
    return {};
}


}
}
