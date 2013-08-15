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
#include <vector>
#include <memory>
#include "util/splay_cache.h"
#include "tokenizers/tokenizers.h"
#include "index/document.h"
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
         * @param docs The untokenized documents to add to the index
         * @param tok The tokenizer to use to tokenize the documents
         */
        inverted_index(std::vector<document> & docs,
                       std::shared_ptr<tokenizers::tokenizer> & tok);

        /**
         * @param index_path The directory containing an already-created index
         */
        inverted_index(const std::string & index_path);

        /**
         * @param t_id The term to search for
         * @return the inverse document frequency of a term
         * @note This function is not const because the cache may be updated
         */
        uint64_t idf(term_id t_id);

        /**
         * @param d_id The document to search for
         * @return the size of the given document (the total number of terms
         * occuring)
         */
        uint64_t doc_size(doc_id d_id) const;

        /**
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         * @note This function is not const because the cache may be updated
         */
        uint64_t term_freq(term_id t_id, doc_id d_id);

    private:
        /**
         * @param t_id The term_id to search for
         * @return the postings data for a given term_id
         * A cache is first searched before the postings file is queried.  This
         * function may be called by inverted_index::term_freq or
         * inverted_index::idf.
         */
        postings_data search_term(term_id t_id);

        /**
         * @param chunk_num The current chunk number of the postings file
         * @param pdata A collection of postings data to write to the chunk
         */
        void write_chunk(uint32_t chunk_num,
                         std::unordered_map<term_id, postings_data> & pdata);

        /**
         * @param docs The documents to add to the inverted index
         * @param tok The tokenizer to use to tokenize the documents
         */
        uint32_t tokenize_docs(std::vector<document> & docs,
                               std::shared_ptr<tokenizers::tokenizer> & tok);

        /**
         * Creates the lexicon file (or "dictionary") which has pointers into
         * the large postings file
         */
        void create_lexicon();

        /**
         * @param num_chunks The total number of chunks to merge together to
         * create the postings file
         */
        void merge_chunks(uint32_t num_chunks);

        /** doc_id -> document name mapping */
        std::unordered_map<doc_id, std::string> _doc_id_mapping;
};

}
}

#endif
