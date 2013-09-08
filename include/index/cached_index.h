/**
 * @file cached_index.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _CACHED_INDEX_H_
#define _CACHED_INDEX_H_

#include "index/document.h"
#include "index/postings_data.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * Decorator class for wrapping indexes with a cache. Like other indexes,
 * you shouldn't construct this directly, but rather use make_index().
 */
template <class Index, template <class, class> class Cache>
class cached_index : public Index {
    public:
        // inherit the constructors
        using Index::Index;

        /**
         * Forwarding constructor: construct the Index part using the
         * config, but then forward the additional arguments to the
         * underlying cache.
         *
         * @param config the configuration that specifies how the index
         *  should be constructed
         * @param args the remaining arguments to send to the Cache
         *  constructor
         */
        template <class... Args>
        cached_index(cpptoml::toml_group & config, Args &&... args);

        using primary_key_type   = typename Index::primary_key_type;
        using secondary_key_type = typename Index::secondary_key_type;

        /**
         * Overload for search_primary() that first attempts to find the
         * result in the cache. Failing that, it will invoke the base class
         * search_primary(), store the result in the cache, and then return
         * the value.
         *
         * @param p_id the primary key to search the postings file for
         */
        virtual std::shared_ptr<postings_data<primary_key_type,
                                              secondary_key_type>>
        search_primary(primary_key_type p_id) const override;
    private:
        /**
         * The internal cache object.
         */
        mutable Cache<primary_key_type,
                      std::shared_ptr<postings_data<primary_key_type,
                                                    secondary_key_type>
                      >
                > cache_;
};

}
}

#include "index/cached_index.tcc"
#endif
