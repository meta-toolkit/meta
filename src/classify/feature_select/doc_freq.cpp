/**
 * @file select_doc_freq.cpp
 */

#include <unordered_map>
#include "classify/feature_select/doc_freq.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::pair;
using std::unordered_map;
using std::unordered_set;

doc_freq::doc_freq(const std::vector<corpus::document> & docs):
    select_simple(docs) { /* nothing */ }

double doc_freq::calc_weight(term_id termID, const class_label & label) const
{
    return term_and_class(termID, label);
}

}
}
