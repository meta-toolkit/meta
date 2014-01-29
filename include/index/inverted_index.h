/**
 * @file inverted_index.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _INVERTED_INDEX_H_
#define _INVERTED_INDEX_H_

#include <queue>
#include <stdexcept>

#include "index/disk_index.h"
#include "index/make_index.h"

namespace meta {

namespace corpus {
class corpus;
class document;
}

namespace index {

template <class>
class chunk_handler;

template <class, class>
class postings_data;
}

}

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
 */
class inverted_index: public disk_index
{
    public:
        class inverted_index_exception;

        using primary_key_type   = term_id;
        using secondary_key_type = doc_id;
        using postings_data_type = postings_data<term_id, doc_id>;
        using index_pdata_type = postings_data<std::string, doc_id>;
        using exception = inverted_index_exception;

    protected:
        /**
         * @param config The toml_group that specifies how to create the
         * index.
         */
        inverted_index(const cpptoml::toml_group & config);

    public:
        /**
         * Move constructs a inverted_index.
         */
        inverted_index(inverted_index&&);

        /**
         * Move assigns a inverted_index.
         */
        inverted_index & operator=(inverted_index&&);

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
        virtual ~inverted_index();

        /**
         * @param doc The document to tokenize
         */
        void tokenize(corpus::document & doc);

        /**
         * @param t_id The term_id to search for
         * @return the postings data for a given term_id
         */
        virtual std::shared_ptr<postings_data_type>
            search_primary(term_id t_id) const;

        /**
         * @param t_id The term to search for
         * @return the document frequency of a term (number of documents it
         * appears in)
         */
        uint64_t doc_freq(term_id t_id) const;

        /**
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         */
        uint64_t term_freq(term_id t_id, doc_id d_id) const;

        /**
         * @return the total number of terms in this index
         */
        uint64_t total_corpus_terms();

        /**
         * @param t_id The specified term
         * @return the number of times the given term appears in the corpus
         */
        uint64_t total_num_occurences(term_id t_id) const;

        /**
         * @return the average document length in this index
         */
        double avg_doc_length();

    private:
        /**
         * This function initializes the disk index. It cannot be part of the
         * constructor since dynamic binding doesn't work in a base class's
         * constructor, so it is invoked from a factory method.
         * @param config_file The configuration file used to create the
         */
        void create_index(const std::string & config_file);

        /**
         * This function loads a disk index from its filesystem
         * representation.
         */
        void load_index();

        /**
         * @param docs The documents to be tokenized
         * @return the number of chunks created
         */
        void tokenize_docs(corpus::corpus * docs,
                           chunk_handler<inverted_index>& handler);

        /**
         * Creates the lexicon file (or "dictionary") which has pointers into
         * the large postings file
         * @param postings_file
         * @param lexicon_file
         */
        void create_lexicon(const std::string & postings_file,
                            const std::string & lexicon_file);

        /**
         * Compresses the large postings file.
         */
        void compress(const std::string & filename, uint64_t num_unique_terms);

        /**
         * The tokenizer used to tokenize documents.
         */
        std::unique_ptr<tokenizers::tokenizer> tokenizer_;

        /**
         * PrimaryKey -> postings location.
         * Each index corresponds to a PrimaryKey (uint64_t).
         */
        std::unique_ptr<util::disk_vector<uint64_t>> _term_bit_locations;

        /** the total number of term occurrences in the entire corpus */
        uint64_t _total_corpus_terms = 0;

    public:
        /**
         * Basic exception for inverted_index interactions.
         */
        class inverted_index_exception: public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };

        /**
         * inverted_index is a friend of the factory method used to create
         * it.
         */
        friend inverted_index make_index<inverted_index>(const std::string &);

        /**
         * Factory method for creating indexes.
         * Usage:
         *
         * ~~~cpp
         * auto idx = index::make_index<derived_index_type>(config_path);
         * ~~~
         *
         * @param config_file The path to the configuration file to be
         *  used to build the index
         * @param args any additional arguments to forward to the
         *  constructor for the chosen index type (usually none)
         * @return A properly initialized index
         */
        template <class Index, class... Args>
        friend Index make_index(const std::string & config_file,
                                Args &&... args);

        /**
         * Factory method for creating indexes that are cached.
         * Usage:
         *
         * ~~~cpp
         * auto idx =
         *     index::make_index<dervied_index_type,
         *                       cache_type>(config_path, other, options);
         * ~~~
         *
         * Other options will be forwarded to the constructor for the
         * chosen cache class.
         *
         * @param config_file the path to the configuration file to be
         *  used to build the index.
         * @param args any additional arguments to forward to the
         *  constructor for the cache class chosen
         * @return A properly initialized, and automatically cached, index.
         */
        template <class Index,
                  template <class, class> class Cache,
                  class... Args>
        friend cached_index<Index, Cache>
        make_index(const std::string & config_file, Args &&... args);
};

}
}

#endif
