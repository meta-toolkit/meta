/**
 * @file score_data.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SCORE_DATA_H_
#define META_SCORE_DATA_H_

#include "meta/config.h"
#include "meta/meta.h"

namespace meta
{

namespace corpus
{
class document;
}

namespace index
{
class inverted_index;
}
}

namespace meta
{
namespace index
{

/**
 * A score_data object contains information needed to evaluate a ranking
 * function. Data is set by the base ranker class as needed, so the derived
 * ranking classes don't make many unncessary calls to the inverted index.
 */
struct score_data
{
    // general info

    /// index queries are running on
    inverted_index& idx;
    /// average document length
    float avg_dl;
    /// total number of documents
    uint64_t num_docs;
    /// total number of terms in the index
    uint64_t total_terms;
    /// the total length of the query (sum of all term weights)
    float query_length;

    // term-based info

    /// doc term id
    term_id t_id;
    /// query term count (or weight in case of feedback)
    float query_term_weight;
    /// number of docs that t_id appears in
    uint64_t doc_count;
    /// number of times t_id appears in corpus
    uint64_t corpus_term_count;

    // document-based info

    /// document id
    doc_id d_id;
    /// number of times the term appears in the current doc
    uint64_t doc_term_count;
    /// total number of terms in the doc
    uint64_t doc_size;
    /// number of unique terms in the doc
    uint64_t doc_unique_terms;

    /**
     * Constructor to initialize most elements.
     * @param p_idx The index that is being used
     * @param p_avg_dl The average doc length in the index
     * @param p_num_docs The number of docs in the index
     * @param p_total_terms The total number of terms in the index
     * @param p_query The current query
     */
    score_data(inverted_index& p_idx, float p_avg_dl, uint64_t p_num_docs,
               uint64_t p_total_terms, float p_length)
        : idx(p_idx), // gcc no non-const ref init from brace init list
          avg_dl{p_avg_dl},
          num_docs{p_num_docs},
          total_terms{p_total_terms},
          query_length{p_length}
    {
        /* nothing */
    }
};
}
}

#endif
