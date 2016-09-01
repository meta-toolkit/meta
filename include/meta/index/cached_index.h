/**
 * @file cached_index.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CACHED_INDEX_H_
#define META_CACHED_INDEX_H_

#include <memory>

#include "meta/config.h"

namespace cpptoml
{
class table;
}

namespace meta
{
namespace index
{

/**
 * Decorator class for wrapping indexes with a cache. Like other indexes,
 * you shouldn't construct this directly, but rather use make_index().
 */
template <class Index, template <class, class> class Cache>
class cached_index : public Index
{
  public:
    /**
     * Forwarding constructor: construct the Index part using the
     * config, but then forward the additional arguments to the
     * underlying cache.
     *
     * @param config the configuration that specifies how the index
     *  should be constructed
     * @param args The remaining arguments to send to the Cache
     *  constructor
     */
    template <class... Args>
    cached_index(const cpptoml::table& config, Args&&... args);

    using primary_key_type = typename Index::primary_key_type;
    using secondary_key_type = typename Index::secondary_key_type;
    using postings_data_type = typename Index::postings_data_type;

    /**
     * Overload for search_primary() that first attempts to find the
     * result in the cache. Failing that, it will invoke the base class
     * search_primary(), store the result in the cache, and then return
     * the value.
     *
     * @param p_id the primary key to search the postings file for
     */
    virtual std::shared_ptr<postings_data_type>
    search_primary(primary_key_type p_id) const override;

    /**
     * Clears the cache for the index. Useful if you're using something
     * like no-evict cache and want to reclaim memory.
     */
    void clear_cache();

  private:
    /**
     * The internal cache object.
     */
    mutable Cache<primary_key_type, std::shared_ptr<postings_data_type>> cache_;
};
}
}

#include "meta/index/cached_index.tcc"
#endif
