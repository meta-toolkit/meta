/**
 * @file inverted_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _INVERTED_INDEX_H_
#define _INVERTED_INDEX_H_

#include <string>
#include "util/splay_cache.h"
#include "index/postings_data.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * The inverted_index class stores information on a corpus indexed by term_ids.
 * Each term_id key is associated with an IDF (inverse document frequency) and
 * per-document frequency (by doc_id). The inverted index also stores the sizes
 * of each document.
 *
 * It is assumed all this information will not fit in memory, so a large
 * postings file containing the (term_id -> each doc_id) information is saved on
 * disk. A lexicon (or "dictionary") is used to store the document size
 * information and contains pointers into the large postings file. It is assumed
 * that the lexicon will fit in memory. The IDF can be calculated by counting
 * the number of doc_ids after a specific term in the postings data.
 *
 * An inverted index consists of the following five files:
 *  - termids.mapping: maps term_id -> string information
 *  - docids.mapping: maps doc_id -> document path
 *  - postings.index: the large file saved on disk for per-document term_id
 *      information
 *  - lexicon.index: the smaller file that contains pointers into the postings
 *      file based on term_id
 *  - docsizes.counts: maps doc_id -> number of terms
 */
class inverted_index
{
    public:
        /**
         * @param t_id The term to search for
         * @return the inverse document frequency of a term
         * @note This function is not const because the cache may be updated
         */
        uint32_t idf(term_id t_id);

        /**
         * @param d_id The document to search for
         * @return the size of the given document (the total number of terms
         * occuring)
         */
        uint32_t doc_size(doc_id d_id) const;

        /**
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         * @note This function is not const because the cache may be updated
         */
        uint32_t term_freq(term_id t_id, doc_id d_id);

    private:
        /**
         * @param t_id The term_id to search for
         * @return the postings data for a given term_id
         * A cache is first searched before the postings file is queried.  This
         * function may be called by inverted_index::term_freq or
         * inverted_index::idf.
         */
        postings_data search_term(term_id t_id);
};

}
}

#endif
