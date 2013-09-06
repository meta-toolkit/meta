/**
 * @file splay_cache.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _SPLAY_CACHE_H_
#define _SPLAY_CACHE_H_

#include "meta.h"

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
         * Creates a splay tree cache with maximum height (depth). This means
         * there are at most \f$2^{max_height + 1} - 1\f$ nodes.
         *
         * @param max_height The maximum height for the splay tree
         */
        splay_cache(uint32_t max_height);

        /**
         * Default move copy-constructor.
         */
        splay_cache(splay_cache && other) = default;

        /**
         * Default move assignment.
         */
        splay_cache & operator=(splay_cache && other) = default;

        /**
         * Frees all objects in the cache
         */
        ~splay_cache();

        /**
         * @param key The key to insert
         * @param value The value to insert
         */
        void insert(const Key & key, const Value & value);

        /**
         * @param key The key to check existance for
         * @return whether the key exists in the cache
         */
        bool exists(const Key & key);

        /**
         * This function assumes exists was called first to verify that the
         * given key actually exists in the cache.
         * @param key The key to find the corresponding value for
         * @return the associated value for the given key
         */
        const Value & find(const Key & key);

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

        uint32_t _max_height;
        node* _root;

        void clear(node* & subroot);
        void insert(node* & subroot, const Key & key, const Value & value, uint32_t depth);
        void find(node* & subroot, const Key & key);

        void rotate_left(node* & subroot);
        void rotate_right(node* & subroot);

    public:

        /**
         * Basic exception for splay_cache interactions.
         */
        class splay_cache_exception: public std::exception
        {
            public:
                
                splay_cache_exception(const std::string & error):
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

#include "caching/splay_cache.tcc"
#endif
