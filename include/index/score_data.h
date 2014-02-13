/**
 * @file score_data.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SCORE_DATA_H_
#define _SCORE_DATA_H_

#include "meta.h"

namespace meta {

namespace corpus {
class document;
}

namespace index {
class inverted_index;
}

}

namespace meta {
namespace index {

/**
 * A score_data object contains information needed to evaluate a ranking
 * function. Data is set by the base ranker class as needed, so the derived
 * ranking classes don't make many unncessary calls to the inverted index.
 */
struct score_data
{
    // general info

    inverted_index & idx;
    double avg_dl;
    uint64_t num_docs;
    uint64_t total_terms;
    const corpus::document & query;

    // term-based info

    term_id t_id;
    uint64_t query_term_count;
    uint64_t doc_count;
    uint64_t corpus_term_count;

    // document-based info

    doc_id d_id;
    uint64_t doc_term_count;
    uint64_t doc_size;
    uint64_t doc_unique_terms;

    score_data(inverted_index & p_idx,
               double p_avg_dl,
               uint64_t p_num_docs,
               uint64_t p_total_terms,
               const corpus::document & p_query):
        idx(p_idx), // gcc no non-const ref init from brace init list
        avg_dl{p_avg_dl},
        num_docs{p_num_docs},
        total_terms{p_total_terms},
        query(p_query) // gcc no non-const ref init from brace init list
    { /* nothing */ }
};

}
}

#endif
