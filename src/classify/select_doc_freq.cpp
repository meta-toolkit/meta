/**
 * @file select_doc_freq.cpp
 */

#include <unordered_map>
#include "classify/select_doc_freq.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using index::document;

select_doc_freq::select_doc_freq(const vector<document> & docs):
    select_simple(docs) { /* nothing */ }

double select_doc_freq::calc_weight(term_id termID, const class_label & label) const
{
    return term_and_class(termID, label);
}

}
}
