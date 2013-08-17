/**
 * @file disk_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DISK_INDEX_H_
#define _DISK_INDEX_H_

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
 * Contains functionality common to inverted_index and forward_index; mainly,
 * creating chunks and merging them together and storing various mappings.
 */
class disk_index
{
    public:
        /**
         * @param index_name The name for this index to be saved as
         * @param tok The tokenizer to use to tokenize the documents
         * tokenizer
         */
        disk_index(const std::string & index_name,
                   std::shared_ptr<tokenizers::tokenizer> & tok);

        /**
         * @param index_path The directory containing an already-created index
         */
        disk_index(const std::string & index_path);

        /**
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         * @note This function is not const because the cache may be updated
         */
        virtual uint64_t term_freq(term_id t_id, doc_id d_id) = 0;

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
         * @param d_id The document to search for
         * @return the size of the given document (the total number of terms
         * occuring)
         */
        uint64_t doc_size(doc_id d_id) const;

        /**
         * @param doc The document to tokenize
         */
        void tokenize(document & doc);

    protected:
        /**
         * This function initializes the disk index. It cannot be part of the
         * constructor since dynamic binding doesn't work in a base class's
         * consturctor. Therefore, deriving classes must call this function in
         * their constructor.
         * @param docs The documents to tokenize
         * @param config_file The configuration file used to create the
         */
        void create_index(std::vector<document> & docs, const std::string & config_file);

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
        virtual uint32_t tokenize_docs(std::vector<document> & docs) = 0;
  
        /** doc_id -> document name mapping */
        std::unordered_map<doc_id, std::string> _doc_id_mapping;

        /** doc_id -> document length mapping */
        std::unordered_map<doc_id, uint64_t> _doc_sizes;

        /** the tokenizer used to tokenize documents in the index */
        std::shared_ptr<tokenizers::tokenizer> _tokenizer;

    private:
        /**
         * @param num_chunks The total number of chunks to merge together to
         * create the postings file
         * @param filename The name for the postings file
         */
        void merge_chunks(uint32_t num_chunks, const std::string & filename);
        
        /**
         * Saves the doc_id -> document name mapping to disk.
         * @param filename The name to save the mapping as
         */
        template <class Key, class Value>
        void save_mapping(const std::unordered_map<Key, Value> & map,
                          const std::string & filename) const;

        /**
         * @param map The map to load information into
         * @param filename The file containing key, value pairs
         */
        template <class Key, class Value>
        void load_mapping(std::unordered_map<Key, Value> & map,
                          const std::string & filename);

        /**
         * @param idx The pointer into the postings file where the wanted
         * PrimaryKey begins
         * @return a postings_data object from the postings file
         */
        postings_data<term_id, doc_id> search_postings(uint64_t idx);

        /**
         * Creates the lexicon file (or "dictionary") which has pointers into
         * the large postings file
         * @param postings_file
         * @param lexicon_file
         */
        void create_lexicon(const std::string & postings_file,
                            const std::string & lexicon_file);

        /** the location of this index */
        std::string _index_name;

        /** term_id -> postings location */
        std::unordered_map<term_id, uint64_t> _term_locations;
        
        /**
         * A pointer to a memory-mapped postings file. It is a pointer because
         * we want to delay the initialization of it until the postings file is
         * created in some cases.
         */
        std::unique_ptr<io::mmap_file> _postings;

        /** cache for recently used postings_data */
        util::splay_cache<term_id, postings_data<term_id, doc_id>> _cache;
 
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
};

}
}

#endif
