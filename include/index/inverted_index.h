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
#include "io/mmap_file.h"
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
class inverted_index
{
    public:
        /**
         * @param index_name The name for this inverted index to be saved as
         * @param docs The untokenized documents to add to the index
         * @param tok The tokenizer to use to tokenize the documents
         * @param config_file The configuration file used to create the
         * tokenizer
         */
        inverted_index(const std::string & index_name,
                       std::vector<document> & docs,
                       std::shared_ptr<tokenizers::tokenizer> & tok,
                       const std::string & config_file);

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

        /**
         * @param t_id The term id to find containing documents from
         * @return a mapping of doc_id -> term occurrence
         */
        const std::unordered_map<doc_id, uint64_t> counts(term_id t_id);

        /**
         * @return the number of documents in this index
         */
        uint64_t num_docs() const;

        /**
         * @return the average document length in this index
         */
        double avg_doc_length();

        /**
         * @param doc_id
         * @return the actual name of this document
         */
        std::string doc_name(doc_id d_id) const;

        /**
         * @param doc The document to tokenize
         */
        void tokenize(document & doc);

    private:
        /**
         * @param t_id The term_id to search for
         * @return the postings data for a given term_id
         * A cache is first searched before the postings file is queried.  This
         * function may be called by inverted_index::term_freq or
         * inverted_index::idf.
         */
        postings_data<term_id, doc_id> search_term(term_id t_id);

        /**
         * @param chunk_num The current chunk number of the postings file
         * @param pdata A collection of postings data to write to the chunk
         */
        void write_chunk(uint32_t chunk_num,
                         std::unordered_map<term_id, postings_data<term_id, doc_id>> & pdata);

        /**
         * @param docs The documents to add to the inverted index
         */
        uint32_t tokenize_docs(std::vector<document> & docs);

        /**
         * Creates the lexicon file (or "dictionary") which has pointers into
         * the large postings file
         * @param postings_file
         * @param lexicon_file
         */
        void create_lexicon(const std::string & postings_file,
                            const std::string & lexicon_file);

        /**
         * Saves the doc_id -> document name mapping to disk.
         * @param filename The name to save the mapping as
         */
        template <class Key, class Value>
        void save_mapping(const std::unordered_map<Key, Value> & map,
                          const std::string & filename) const;

        /**
         * @param num_chunks The total number of chunks to merge together to
         * create the postings file
         * @param filename The name for the postings file
         */
        void merge_chunks(uint32_t num_chunks, const std::string & filename);

        /**
         * @param map The map to load information into
         * @param filename The file containing key, value pairs
         */
        template <class Key, class Value>
        void load_mapping(std::unordered_map<Key, Value> & map,
                          const std::string & filename);

        /**
         * @param idx The pointer into the postings file where the wanted
         * term_id begins
         * @return a postings_data object from the postings file
         */
        postings_data<term_id, doc_id> search_postings(uint64_t idx);

        /** the location of this index */
        std::string _index_name;

        /** doc_id -> document name mapping */
        std::unordered_map<doc_id, std::string> _doc_id_mapping;

        /** doc_id -> document length mapping */
        std::unordered_map<doc_id, uint64_t> _doc_sizes;

        /** term_id -> postings location */
        std::unordered_map<term_id, uint64_t> _term_locations;

        /** cache for recently used postings_data */
        util::splay_cache<term_id, postings_data<term_id, doc_id>> _cache;

        /**
         * A pointer to a memory-mapped postings file. It is a pointer because
         * we want to delay the initialization of it until the postings file is
         * created in some cases.
         */
        std::unique_ptr<io::mmap_file> _postings;

        /** average document length in the inverted_index */
        double _avg_dl;

        /** the tokenizer used to tokenize documents in the index */
        std::shared_ptr<tokenizers::tokenizer> _tokenizer;

    public:
        /**
         * Basic exception for inverted_index interactions.
         */
        class inverted_index_exception: public std::exception
        {
            public:
                
                inverted_index_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
                std::string _error;
        };
};

}
}

#endif
