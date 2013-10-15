/**
 * @file disk_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DISK_INDEX_H_
#define _DISK_INDEX_H_

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "corpus/corpus.h"
#include "tokenizers/all.h"
#include "index/cached_index.h"
#include "index/postings_data.h"
#include "io/compressed_file_reader.h"
#include "io/mmap_file.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * Contains functionality common to inverted_index and forward_index; mainly,
 * creating chunks and merging them together and storing various mappings.
 */
template <class PrimaryKey, class SecondaryKey>
class disk_index
{
    public:
        using primary_key_type = PrimaryKey;
        using secondary_key_type = SecondaryKey;
        using PostingsData = postings_data<PrimaryKey, SecondaryKey>;
    protected:
        /**
         * @param config The toml_group that specifies how to create the
         * index.
         * @param index_path The location to store the index on disk.
         */
        disk_index(const cpptoml::toml_group & config,
                   const std::string & index_path);

        /**
         * Move constructs a disk_index.
         * @param other The disk_index to move into this one.
         */
        disk_index(disk_index && other) = default;

        /**
         * Move assigns a disk_index.
         * @param other The disk_index to move into this one.
         */
        disk_index & operator=(disk_index && other) = default;

        /**
         * disk_index may not be copy-constructed.
         */
        disk_index(const disk_index &) = delete;

        /**
         * disk_index may not be copy-assigned.
         */
        disk_index & operator=(const disk_index &) = delete;

    public:
        /**
         * Default destructor.
         */
        virtual ~disk_index() = default;

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
         * occuring)
         */
        double doc_size(doc_id d_id) const;

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
         * @param p_id The PrimaryKey id to search for
         * @return the postings data for a given PrimaryKey
         * A cache is first searched before the postings file is queried.
         */
        virtual std::shared_ptr<PostingsData>
        search_primary(PrimaryKey p_id) const;

    protected:
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
         * @param pdata A collection of postings data to write to the chunk
         */
        void write_chunk(uint32_t chunk_num, std::vector<PostingsData> & pdata);

        /**
         * Saves any arbitrary mapping to the disk.
         * @param map The map to read key, value pairs from
         * @param filename The name to save the mapping as
         */
        template <class Key, class Value>
        static void save_mapping(const util::invertible_map<Key, Value> & map,
                                 const std::string & filename);

        /**
         * Vector-specific version of save_mapping: saves any arbitrary mapping
         * to the disk.
         * @param map The map to read key, value pairs from
         * @param filename The name to save the mapping as
         */
        template <class T>
        static void save_mapping(const std::vector<T> & vec,
                                 const std::string & filename);

        /**
         * @param map The map to load information into
         * @param filename The file containing key, value pairs
         */
        template <class Key, class Value>
        static void load_mapping(util::invertible_map<Key, Value> & map,
                                 const std::string & filename);

        /**
         * Vector-specific version of load_mapping.
         * @param vec The vector to load information into
         * @param filename The file containing the vector's data.
         */
        template <class T>
        static void load_mapping(std::vector<T> & vec,
                                 const std::string & filename);

        /**
         * @param docs The documents to add to the inverted index
         */
        virtual uint32_t tokenize_docs(corpus::corpus * docs) = 0;

        /**
         * @param d_id The document
         * @return the numerical label_id for a given document's label
         */
        label_id label_id_from_doc(doc_id d_id) const;

        template <class Derived, uint32_t MaxSize = 1000>
        class chunk_handler {
            uint32_t chunk_size_{0};
            disk_index * idx_;
            std::atomic<uint32_t> & chunk_num_;

            public:
                chunk_handler(disk_index * idx,
                              std::atomic<uint32_t> & chunk_num)
                    : idx_{idx}, chunk_num_{chunk_num}
                { /* nothing */ }

                void operator()(const corpus::document & doc) {
                    static_cast<Derived *>(this)->handle_doc(doc);
                    if (++chunk_size_ % MaxSize == 0)
                        write_chunk();
                }

                void write_chunk() {
                    auto vec = static_cast<Derived *>(this)->chunk();
                    idx_->write_chunk(chunk_num_.fetch_add(1), vec);
                    chunk_size_ = 0;
                }
        };

        /**
         * @param docs the documents to create chunks for.
         */
        template <class ChunkHandler>
        uint32_t create_chunks(corpus::corpus * docs);

        /**
         * doc_id -> document path mapping.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::vector<std::string> _doc_id_mapping;

        /**
         * doc_id -> document length mapping.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::vector<double> _doc_sizes;

        /** the tokenizer used to tokenize documents in the index */
        std::unique_ptr<tokenizers::tokenizer> _tokenizer;

        /** the mapping of (actual -> compressed id) */
        util::invertible_map<uint64_t, uint64_t> _compression_mapping;

        /**
         * Maps which class a document belongs to (if any).
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::vector<class_label> _labels;

        /**
         * Holds how many unique terms there are per-document. This is sort of
         * like an inverse IDF. For a forward_index, this field is certainly
         * redundant, though it can save querying the postings file.
         * Each index corresponds to a doc_id (uint64_t).
         */
        std::vector<uint64_t> _unique_terms;

        /** the total number of term occurrences in the entire corpus */
        uint64_t _total_corpus_terms;

    private:
        /**
         * @param num_chunks The number of chunks to be merged
         * @param filename The name for the postings file
         */
        void merge_chunks(uint32_t num_chunks, const std::string & filename);

        /**
         * Creates the lexicon file (or "dictionary") which has pointers into
         * the large postings file
         * @param postings_file
         * @param lexicon_file
         */
        void create_lexicon(const std::string & postings_file,
                            const std::string & lexicon_file);

        /**
         * Calculates frequency info from the postings file to make the best
         * compression mapping.
         * @param filename The filename of the postings file
         */
        void calc_compression_mapping(const std::string & filename);

        /**
         * Compresses the large postings file.
         */
        void compress(const std::string & filename);

        /**
         * Initializes the _label_ids member.
         */
        void set_label_ids();

        /** the location of this index */
        std::string _index_name;

        /**
         * PrimaryKey -> postings location.
         * Each index corresponds to a PrimaryKey (uint64_t).
         */
        std::vector<uint64_t> _term_bit_locations;

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

    public:
        /**
         * Basic exception for disk_index interactions.
         */
        class disk_index_exception: public std::exception
        {
            public:

                disk_index_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
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

#include "index/disk_index.tcc"
#endif
