/**
 * @file splay_cache.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_SPLAY_CACHE_H_
#define META_SPLAY_CACHE_H_

#include <memory>
#include <mutex>
#include <vector>

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

namespace meta
{
namespace caching
{

/**
 * A splay_cache is a fixed-size splay tree for cache operations.
 */
template <class Key, class Value>
class splay_cache
{
  public:
    /**
     * Creates a splay tree cache with maximum size (or unlimited size, if
     * no size is given).
     *
     * @param max_size The maximum number of nodes that will be in the splay
     * tree
     */
    splay_cache(uint64_t max_size = std::numeric_limits<uint64_t>::max());

    /**
     * splay_cache can be move constructed
     */
    splay_cache(splay_cache&&);

    /**
     * splay_cache can be move assigned
     * @return the current splay_cache
     */
    splay_cache& operator=(splay_cache&&);

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
    void insert(const Key& key, const Value& value);

    /**
     * @param key The key to find the corresponding value for
     * @return an optional containing the associated value for the given
     * key, if found
     */
    util::optional<Value> find(const Key& key);

    /**
     * @return the number of elements in the cache
     */
    uint64_t size() const;

    /**
     * Empties the cache.
     */
    void clear();

  private:
    /** disallow copying */
    splay_cache(const splay_cache&) = delete;

    /** disallow assignment */
    splay_cache& operator=(const splay_cache&) = delete;

    /**
     * One node in the splay tree contains pointers to children and the
     * templated (key, value) pair.
     */
    struct node
    {
        /// pointer to the left child
        node* left;
        /// pointer to the right child
        node* right;
        /// the key
        Key key;
        /// the value
        Value value;

        /**
         * Constructs a new leaf node with the given key and value pair
         * @param new_key The desired key
         * @param new_value The desired value
         */
        node(const Key& new_key, const Value& new_value)
            : left(nullptr), right(nullptr), key(new_key), value(new_value)
        {
            /* nothing */
        }
    };

    /// the current size of the cache
    uint64_t size_;
    /// the maximum allowed size for the cache
    uint64_t max_size_;
    /// the root of the tree
    node* root_;
    /// the mutex that synchronizes access to the cache
    mutable std::mutex mutables_;

    /**
     * Deletes everything at this subroot and below.
     * @param subroot The root of the subtree to clear
     */
    void clear(node*& subroot);

    /**
     * Inserts the given key, value pair into the tree rooted at subroot.
     *
     * @param subroot The root of the subtree to insert into
     * @param key
     * @param value
     */
    void insert(node*& subroot, const Key& key, const Value& value);

    /**
     * Replaces the key, value pair contained in the node pointed to by
     * subroot with the given key, value pair.
     *
     * @param subroot
     * @param key
     * @param value
     */
    void replace(node* subroot, const Key& key, const Value& value);

    /**
     * "Finds" the given key in the tree rooted at subroot. This function
     * does not return anything because it splays the desired value to the
     * root, so the public find() can simply return the root after calling
     * this function.
     *
     * @param subroot
     * @param key
     */
    void find(node*& subroot, const Key& key);

    /**
     * Rotates the tree rooted at subroot to the left.
     * @param subroot
     */
    void rotate_left(node*& subroot);

    /**
     * Rotates the tree rooted at subroot to the right.
     * @param subroot
     */
    void rotate_right(node*& subroot);
};

/**
 * Basic exception for splay_cache interactions.
 */
class splay_cache_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}

#include "meta/caching/splay_cache.tcc"
#endif
