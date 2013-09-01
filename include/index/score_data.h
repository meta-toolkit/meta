/**
 * @file score_data.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 * 
 * @author Sean Massung
 */

#ifndef _SCORE_DATA_H_
#define _SCORE_DATA_H_

#include "meta.h"
#include "index/document.h"

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
    const document & query;

    // term-based info

    term_id t_id;
    uint64_t query_term_count;
    uint64_t idf;

    // document-based info

    doc_id d_id;
    uint64_t doc_term_count;
    uint64_t doc_size;

    score_data(inverted_index & p_idx,
               double p_avg_dl,
               uint64_t p_num_docs,
               uint64_t p_total_terms,
               const document & p_query):
        idx{p_idx},
        avg_dl{p_avg_dl},
        num_docs{p_num_docs},
        total_terms{p_total_terms},
        query{p_query}
    { /* nothing */ }
};

}
}

#endif
