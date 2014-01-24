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

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include "caching/all.h"
#include "corpus/corpus.h"
#include "index/cached_index.h"
#include "index/chunk.h"
#include "index/make_index.h"
#include "index/postings_data.h"
#include "index/vocabulary_map.h"
#include "io/compressed_file_reader.h"
#include "io/mmap_file.h"
#include "meta.h"
#include "tokenizers/all.h"
#include "util/disk_vector.h"
#include "util/sqlite_map.h"

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
        class inverted_index_exception;

        using primary_key_type   = term_id;
        using secondary_key_type = doc_id;
        using postings_data_type = postings_data<term_id, doc_id>;
        using exception = inverted_index_exception;

    private:
        using index_pdata_type = postings_data<std::string, doc_id>;

    protected:
        /**
         * @param config The toml_group that specifies how to create the
         * index.
         */
        inverted_index(const cpptoml::toml_group & config);

    public:
        /**
         * Move constructs a inverted_index.
         * @param other The inverted_index to move into this one.
         */
        inverted_index(inverted_index &&) = default;

        /**
         * Move assigns a inverted_index.
         * @param other The inverted_index to move into this one.
         */
        inverted_index & operator=(inverted_index &&) = default;

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
         * @return the name of this index
         */
        std::string index_name() const;

        /**
         * @return the number of documents in this index
         */
        uint64_t num_docs() const;

        /**
         * @param doc_id
         * @return the actual name of this document
         */
        std::string doc_name(doc_id d_id) const;

        /**
         * @param doc_id
         * @return the path to the file containing this document
         */
        std::string doc_path(doc_id d_id) const;

        /**
         * @return a vector of doc_ids that are contained in this index
         */
        std::vector<doc_id> docs() const;

        /**
         * @param d_id The document to search for
         * @return the size of the given document (the total number of terms
         * occurring)
         */
        uint64_t doc_size(doc_id d_id) const;

        /**
         * @param doc The document to tokenize
         */
        void tokenize(corpus::document & doc);

        /**
         * @param d_id The doc id to find the class label for
         * @return the label of the class that the document belongs to, or an
         * empty string if a label was not assigned
         */
        class_label label(doc_id d_id) const;

        /**
         * @param l_id The id of the class label in question
         * @return the integer label id of a document
         */
        class_label class_label_from_id(label_id l_id) const;

        /**
         * @param d_id
         * @return the number of unique terms in d_id
         */
        uint64_t unique_terms(doc_id d_id) const;

        /**
         * @return the number of unique terms in the index
         */
        uint64_t unique_terms() const;

        /**
         * @param term
         * @return the term_id associated with the parameter
         */
        term_id get_term_id(const std::string & term);

        /**
         * @param t_id The term_id to search for
         * @return the postings data for a given term_id
         */
        virtual std::shared_ptr<postings_data_type>
            search_primary(term_id t_id) const;

        /**
         * @param t_id The term to search for
         * @return the inverse document frequency of a term
         */
        uint64_t idf(term_id t_id) const;

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

        /**
         * inverted_index is a friend of the factory method used to create
         * it.
         */
        friend inverted_index make_index<inverted_index>(const std::string &);

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
         * @param chunk_num The id of the chunk to write
         * @param pdata A collection of postings data to write to the chunk.
         */
        void write_chunk(uint32_t chunk_num,
                         std::vector<index_pdata_type> & pdata);

        /**
         * @param d_id The document
         * @return the numerical label_id for a given document's label
         */
        label_id label_id_from_doc(doc_id d_id) const;

        /**
         * @param docs The documents to be tokenized
         * @return the number of chunks created
         */
        void tokenize_docs(corpus::corpus * docs);

        /**
         * The chunk handler for inverted indexes.
         */
        class chunk_handler {
            /** the current in-memory chunk */
            std::unordered_set<index_pdata_type> pdata_;

            /** the current size of the in-memory chunk */
            uint64_t chunk_size_{0};

            void flush_chunk();

            const static uint64_t constexpr max_size = 1024*1024*128; // 128 MB

            /** a back-pointer to the index this handler is operating on */
            inverted_index * idx_;

            /** the current chunk number */
            std::atomic<uint32_t> & chunk_num_;

            public:
                /**
                 * Creates a new handler on the given index, using the
                 * given atomic to keep track of the current chunk number.
                 */
                chunk_handler(inverted_index * idx,
                              std::atomic<uint32_t> & chunk_num)
                    : idx_{idx}, chunk_num_{chunk_num}
                { /* nothing */ }

                /**
                 * Handler for when a given doc has been successfully
                 * tokenized.
                 */
                void operator()(const corpus::document & doc);

                /**
                 * Destroys the handler, writing to disk any chunk data
                 * still resident in memory.
                 */
                ~chunk_handler();
        };

        /**
         * doc_id -> document path mapping.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::unique_ptr<util::sqlite_map<doc_id, std::string,
                                         caching::default_dblru_cache>>
        _doc_id_mapping;

        /**
         * doc_id -> document length mapping.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::unique_ptr<util::disk_vector<double>> _doc_sizes;

        /** the tokenizer used to tokenize documents in the index */
        std::unique_ptr<tokenizers::tokenizer> _tokenizer;

        /**
         * Maps which class a document belongs to (if any).
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::unique_ptr<util::disk_vector<label_id>> _labels;

        /**
         * Holds how many unique terms there are per-document. This is sort of
         * like an inverse IDF. For a forward_index, this field is certainly
         * redundant, though it can save querying the postings file.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::unique_ptr<util::disk_vector<uint64_t>> _unique_terms;

        /**
         * Maps string terms to term_ids.
         */
        std::unique_ptr<vocabulary_map> _term_id_mapping;

        /** the total number of term occurrences in the entire corpus */
        uint64_t _total_corpus_terms = 0;

        /**
         * @param num_chunks The number of chunks to be merged
         * @param filename The name for the postings file
         */
        uint64_t merge_chunks(const std::string & filename);

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
         * @param lbl the string class label to find the id for
         * @return the label_id of a class_label, creating a new one if
         * necessary
         */
        label_id get_label_id(const class_label & lbl);

        /** the location of this index */
        std::string _index_name;

        /**
         * PrimaryKey -> postings location.
         * Each index corresponds to a PrimaryKey (uint64_t).
         */
        std::unique_ptr<util::disk_vector<uint64_t>> _term_bit_locations;

        /**
         * A pointer to a memory-mapped postings file. It is a pointer because
         * we want to delay the initialization of it until the postings file is
         * created in some cases.
         */
        std::unique_ptr<io::mmap_file> _postings;

        /**
         * assigns an integer to each class label (used for liblinear and slda
         * mappings)
         */
        util::invertible_map<class_label, label_id> _label_ids;

        /** mutex for thread-safe operations */
        std::unique_ptr<std::mutex> _mutex{new std::mutex};

        /** mutex for accessing the priority_queue of chunks */
        std::unique_ptr<std::mutex> _queue_mutex{new std::mutex};

        /** used to select which chunk to merge next */
        std::priority_queue<chunk<std::string, secondary_key_type>> _chunks;

    public:
        /**
         * Basic exception for inverted_index interactions.
         */
        class inverted_index_exception: public std::exception
        {
            public:

                inverted_index_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what() const throw()
                {
                    return _error.c_str();
                }

            private:
                std::string _error;
        };

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
