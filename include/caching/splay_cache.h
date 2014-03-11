/**
 * @file splay_cache.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SPLAY_CACHE_H_
#define META_SPLAY_CACHE_H_

#include <memory>
#include <mutex>
#include <vector>

#include "meta.h"
#include "util/optional.h"

namespace meta {
namespace caching {

/**
 * A splay_cache is a fixed-height splay tree for cache operations
 */
template <class Key, class Value>
class splay_cache
{
    public:
        /**
         * Creates a splay tree cache with maximum size (or not, if no size is
         * given).
         *
         * @param max_size The maximum number of nodes that will be in the splay
         * tree
         */
        splay_cache(uint64_t max_size = std::numeric_limits<uint64_t>::max());

        /**
         * splay_cache can be move constructed
         */
        splay_cache(splay_cache && other);

        /**
         * splay_cache can be move assigned
         */
        splay_cache & operator=(splay_cache && other);

        /**
         * Frees all objects in the cache
         */
        ~splay_cache();

        /**
         * @param key The key to insert
         * @param value The value to insert
         *
         * If the key exists in the map, it will be overwritten.
         */
        void insert(const Key & key, const Value & value);

        /**
         * @param key The key to find the corresponding value for
         * @return the associated value for the given key
         */
        util::optional<Value> find(const Key & key);

        /**
         * @return the number of elements in the cache
         */
        uint64_t size() const;

        /**
         * Adds a listener for when key/value pairs are removed. Useful for
         * implementing write-back caches.
         */
        template <class Functor>
        void on_drop(Functor && fun);

        /**
         * Empties the cache.
         */
        void clear();

    private:

        /** disallow copying */
        splay_cache(const splay_cache & other) = delete;

        /** disallow assignment */
        splay_cache & operator=(const splay_cache & rhs) = delete;

        struct node
        {
            node* left;
            node* right;
            Key key;
            Value value;

            node(const Key & new_key, const Value & new_value):
                left(nullptr), right(nullptr), key(new_key), value(new_value)
            { /* nothing */ }
        };

        uint64_t size_;
        uint64_t max_size_;
        node* root_;
        mutable std::mutex mutables_;

        std::vector<std::function<void(const Key & key, const Value & value)>>
        drop_callbacks_;

        void clear(node* & subroot);
        void insert(node* & subroot, const Key & key, const Value & value);
        void replace(node* subroot, const Key & key, const Value & value);
        void find(node* & subroot, const Key & key);

        void rotate_left(node* & subroot);
        void rotate_right(node* & subroot);

    public:

        /**
         * Basic exception for splay_cache interactions.
         */
        class splay_cache_exception: public std::runtime_error
        {
            public:
                using std::runtime_error::runtime_error;
        };

};

}
}

#include "caching/splay_cache.tcc"
#endif
