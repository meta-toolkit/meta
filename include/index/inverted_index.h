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
#include "index/disk_index.h"
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
 *  - config.toml: saves the tokenizer configuration
 */
class inverted_index: public disk_index<term_id, doc_id>
{
    protected:
        /**
         * @param config The toml_group that specifies how to create the
         * index.
         */
        inverted_index(const cpptoml::toml_group & config);

    public:
        using PostingsData = postings_data<term_id, doc_id>;

        /**
         * Move constructs an inverted index.
         * @param other The inverted_index to move into this one.
         */
        inverted_index(inverted_index && other) = default;

        /**
         * Move assigns an inverted_index.
         * @param other The inverted_index to move into this one.
         */
        inverted_index & operator=(inverted_index && other) = default;

        /**
         * inverted_index may not be copy-constructed.
         */
        inverted_index(const inverted_index &) = delete;

        /**
         * inverted_index may not be copy-assigned.
         */
        inverted_index & operator=(const inverted_index &) = delete;

        /**
         * Default destructor.
         */
        virtual ~inverted_index() = default;

        /**
         * @param t_id The term to search for
         * @return the inverse document frequency of a term
         */
        uint64_t idf(term_id t_id) const;

        /**
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         */
        double term_freq(term_id t_id, doc_id d_id) const;

        /**
         * @return the total number of terms in this index
         */
        uint64_t total_corpus_terms();

        /**
         * @param t_id The specified term
         * @return the number of times the given term appears in the corpus
         */
        double total_num_occurences(term_id t_id) const;

        /**
         * @param t_id The term id to find containing documents from
         * @return a mapping of doc_id -> term occurrence
         */
        const std::vector<std::pair<doc_id, double>> counts(term_id t_id) const;

        /**
         * @return the average document length in this index
         */
        double avg_doc_length();

        /**
         * inverted_index is a friend of the factory method used to create
         * it.
         */
        friend inverted_index make_index<inverted_index>(const std::string &);

    protected:
        /**
         * @param docs The documents to add to the inverted index
         */
        uint32_t tokenize_docs(const std::unique_ptr<corpus::corpus> & docs);

    private:

        /**
         * @param pdata
         * @return a vector version of postings data for writing chunks
         */
        static std::vector<PostingsData> to_vector(
                std::unordered_map<term_id, PostingsData> & pdata);

        /** average document length in the inverted_index */
        double _avg_dl;

        /** the total number of term occurences in the entire corpus */
        uint64_t _total_corpus_terms;

};

}
}

#endif
