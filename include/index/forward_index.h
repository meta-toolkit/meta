/**
 * @file forward_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _FORWARD_INDEX_H_
#define _FORWARD_INDEX_H_

#include <stdexcept>

#include "index/disk_index.h"
#include "index/make_index.h"
#include "util/disk_vector.h"
#include "meta.h"

namespace meta {
namespace corpus {
class corpus;
}

namespace index {
template <class, class>
class postings_data;
}
}

namespace meta {
namespace index {

/**
 * The forward_index stores information on a corpus by doc_ids.  Each doc_id key
 * is associated with a distribution of term_ids or term "counts" that occur in
 * that particular document.
 */
class forward_index: public disk_index
{
   public:
    class forward_index_exception;

    using primary_key_type   = doc_id;
    using secondary_key_type = term_id;
    using postings_data_type = postings_data<doc_id, term_id>;
    using inverted_pdata_type = postings_data<term_id, doc_id>;
    using index_pdata_type = postings_data_type;
    using exception = forward_index_exception;

   protected:
    /**
     * @param config The toml_group that specifies how to create the
     * index.
     */
    forward_index(const cpptoml::toml_group &config);

   public:
    /**
     * Move constructs a forward_index.
     * @param other The forward_index to move into this one.
     */
    forward_index(forward_index &&other) = default;

    /**
     * Move assigns a forward_index.
     * @param other The forward_index to move into this one.
     */
    forward_index &operator=(forward_index &&other) = default;

    /**
     * forward_index may not be copy-constructed.
     */
    forward_index(const forward_index &) = delete;

    /**
     * forward_index may not be copy-assigned.
     */
    forward_index &operator=(const forward_index &) = delete;

    /**
     * Default destructor.
     */
    virtual ~forward_index() = default;

    /**
     * @return the name of this index
     */
    std::string index_name() const;

    /**
     * @param d_id The doc_id to search for
     * @return the postings data for a given doc_id
     */
    virtual std::shared_ptr<postings_data_type>
        search_primary(doc_id d_id) const;

    /**
     * @param d_id The document id of the doc to convert to liblinear format
     * @return the string representation liblinear format
     */
    std::string liblinear_data(doc_id d_id) const;

    /**
     * @return the number of unique terms in the index
     */
    virtual uint64_t unique_terms() const override;

   private:
    /** the name of this index on disk */
    std::string _index_name;

    /**
     * Initializes this index's metadata structures.
     */
    void init_metadata();

    /**
     * This function loads a disk index from its filesystem
     * representation.
     */
    void load_index();

    /**
     * This function initializes the forward index.
     * @param config_file The configuration file used to create the index
     */
    void create_index(const std::string & config_file);

    /**
     * @param config the configuration settings for this index
     */
    void create_libsvm_postings(const cpptoml::toml_group& config);

    /**
     */
    void create_libsvm_metadata();

    /**
     * @param config the configuration settings for this index
     */
    void uninvert(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     */
    void create_uninverted_metadata(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     * @return whether this index will be based off of a single
     * libsvm-formatted corpus file
     */
    bool is_libsvm_format(const cpptoml::toml_group& config) const;

    /**
     * Merges chunks from uninverting into a single postings_data file.
     * @param num_chunks The number of chunks that need to be merged together
     * into one postings file
     */
    void merge_chunks(uint32_t num_chunks);

    /**
     * @param chunk_num The id of the chunk to write
     * @param pdata A collection of postings data to write to the chunk.
     */
    void write_chunk(uint32_t chunk_num, std::vector<index_pdata_type>& pdata);

    /**
     * Calculates which documents start at which bytes in the postings file.
     */
    void set_doc_byte_locations();

    /**
     * Converts postings.index into a libsvm formatted file
     */
    void compressed_postings_to_libsvm(const std::string & filename);

    /** the total number of unique terms if _term_id_mapping is unused */
    uint64_t _total_unique_terms;

    /** doc_id -> postings file byte location */
    std::unique_ptr<util::disk_vector<uint64_t>> _doc_byte_locations;

    public:
        /**
         * Basic exception for forward_index interactions.
         */
        class forward_index_exception: public std::runtime_error
        {
            public:
               using std::runtime_error::runtime_error;
        };

        /**
         * forward_index is a friend of the factory method used to create
         * it.
         */
        friend forward_index make_index<forward_index>(const std::string &);

        /**
         * Factory method for creating indexes.
         */
        template <class Index, class... Args>
        friend Index make_index(const std::string & config_file,
                                Args &&... args);

        /**
         * Factory method for creating indexes that are cached.
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
