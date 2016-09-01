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

#include "meta/config.h"
#include "meta/index/disk_index.h"
#include "meta/index/make_index.h"
#include "meta/index/postings_stream.h"
#include "meta/learn/instance.h"
#include "meta/meta.h"
#include "meta/util/disk_vector.h"
#include "meta/util/optional.h"

namespace meta
{
namespace corpus
{
class corpus;
}

namespace index
{
template <class, class, class>
class postings_data;
}
}

namespace meta
{
namespace index
{
/**
 * Basic exception for forward_index interactions.
 */
class forward_index_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

/**
 * The forward_index stores information on a corpus by doc_ids.  Each doc_id key
 * is associated with a distribution of term_ids or term "counts" that occur in
 * that particular document.
 */
class forward_index : public disk_index
{
  public:
    /**
     * forward_index is a friend of the factory method used to create it.
     */
    template <class Index, class... Args>
    friend std::shared_ptr<Index> make_index(const cpptoml::table& config,
                                             Args&&... args);

    /**
     * forward_index is a friend of the factory method used to create it.
     */
    template <class Index, class... Args>
    friend std::shared_ptr<Index> make_index(const cpptoml::table& config,
                                             corpus::corpus& docs,
                                             Args&&... args);
    /**
     * forward_index is a friend of the factory method used to create cached
     * versions of it.
     */
    template <class Index, template <class, class> class Cache, class... Args>
    friend std::shared_ptr<cached_index<Index, Cache>>
    make_index(const cpptoml::table& config_file, Args&&... args);

    using primary_key_type = doc_id;
    using secondary_key_type = term_id;
    using postings_data_type = postings_data<doc_id, term_id, double>;
    using inverted_pdata_type = postings_data<term_id, doc_id, uint64_t>;
    using index_pdata_type = postings_data<doc_id, term_id, uint64_t>;
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
     * @param d_id The doc_id to search for
     * @return the postings stream for a given doc_id
     */
    util::optional<postings_stream<term_id, double>>
    stream_for(doc_id d_id) const;

    /**
     * @param d_id The document id of the doc to convert to liblinear format
     * @return the string representation liblinear format
     */
    std::string liblinear_data(doc_id d_id) const;

    /**
     * @return the number of unique terms in the index
     */
    virtual uint64_t unique_terms() const override;

    /**
     * @param doc The document to tokenize
     * @return the analyzed version of the document as a feature vector
     */
    learn::feature_vector tokenize(const corpus::document& doc);

  private:
    /**
     * Loads a forward index from its filesystem representation.
     */
    void load_index();

    /**
     * Initializes the forward index; it is called by the make_index factory
     * function.
     * @param config The configuration to be used
     * @param docs A corpus object of documents to index
     */
    void create_index(const cpptoml::table& config, corpus::corpus& docs);

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
