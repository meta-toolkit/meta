/**
 * @file forward_index.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_FORWARD_INDEX_H_
#define META_FORWARD_INDEX_H_

#include <stdexcept>

#include "index/disk_index.h"
#include "index/make_index.h"
#include "util/disk_vector.h"
#include "meta.h"

namespace meta
{
namespace corpus
{
class corpus;
}

namespace index
{
template <class, class>
class postings_data;
}
}

namespace meta
{
namespace index
{

/**
 * The forward_index stores information on a corpus by doc_ids.  Each doc_id key
 * is associated with a distribution of term_ids or term "counts" that occur in
 * that particular document.
 */
class forward_index : public disk_index
{
  public:
    /**
     * Basic exception for forward_index interactions.
     */
    class forward_index_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * forward_index is a friend of the factory method used to create
     * it.
     */
    template <class Index, class... Args>
    friend std::shared_ptr<Index> make_index(const std::string& config_file,
                                             Args&&... args);

    /**
     * forward_index is a friend of the factory method used to create
     * cached versions of it.
     */
    template <class Index, template <class, class> class Cache, class... Args>
    friend std::shared_ptr<cached_index<Index, Cache>>
        make_index(const std::string& config_file, Args&&... args);

    using primary_key_type = doc_id;
    using secondary_key_type = term_id;
    using postings_data_type = postings_data<doc_id, term_id>;
    using inverted_pdata_type = postings_data<term_id, doc_id>;
    using index_pdata_type = postings_data_type;
    using exception = forward_index_exception;

  protected:
    /**
     * @param config The table that specifies how to create the
     * index.
     */
    forward_index(const cpptoml::table& config);

  public:
    /**
     * Move constructs a forward_index.
     */
    forward_index(forward_index&&);

    /**
     * Move assigns a forward_index.
     * @param other The forward_index to move into this one.
     */
    forward_index& operator=(forward_index&&);

    /**
     * forward_index may not be copy-constructed.
     */
    forward_index(const forward_index&) = delete;

    /**
     * forward_index may not be copy-assigned.
     */
    forward_index& operator=(const forward_index&) = delete;

    /**
     * Default destructor.
     */
    virtual ~forward_index();

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
    /**
     * This function loads a disk index from its filesystem
     * representation.
     */
    void load_index();

    /**
     * This function initializes the forward index.
     * @param config_file The configuration file used to create the index
     */
    void create_index(const std::string& config_file);

    /**
     * @return whether this index contains all necessary files
     */
    bool valid() const;

    /// Forward declare the implementation
    class impl;
    /// Implementation of this index
    util::pimpl<impl> fwd_impl_;
};
}
}

#endif
