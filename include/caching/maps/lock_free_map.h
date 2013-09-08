/**
 * @file lock_free_map.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Chase Geigle
 */

#ifndef _LOCK_FREE_MAP_H_
#define _LOCK_FREE_MAP_H_

#include <memory>
#include <unordered_map>

#include "util/optional.h"

namespace meta {
namespace caching {

/**
 * A lock_free_map is an implementation of a thread-safe unordered_map
 * that is "lock free" (in the sense that it does not use mutexes). This
 * map guarantees fast, unblocking access for reads, but at the potential
 * expense of writes, which may be very slow and may block on readers.
 */
template <class Key, class Value>
class lock_free_map {
    public:
        /**
         * Default constructor for lock_free_map. Invokes the default
         * constructor for std::unordered_map internally.
         */
        lock_free_map();

        /**
         * lock_free_map may be copy constructed
         */
        lock_free_map(const lock_free_map &) = default;

        /**
         * lock_free_map may be move constructed
         */
        lock_free_map(lock_free_map &&);

        /**
         * lock_free_map may be copy or move assigned (unifying assignment
         * op)
         */
        lock_free_map & operator=(lock_free_map rhs);

        /**
         * lock_free_map has a default destructor
         */
        ~lock_free_map() = default;

        /**
         * Swaps the current lock_free_map with the given one
         * @param other the map to swap with
         */
        void swap(lock_free_map & other);

        /**
         * Inserts a given (key, value) pair into the hash table.
         * @param key the key to be inserted
         * @param value the value to be inserted
         */
        void insert(const Key & key, const Value & value);

        /**
         * Inserts a (key, value) pair into the hash table, using perfect
         * forwarding to std::unordered_map's emplace() method.
         */
        template <class... Args>
        void emplace(Args &&... args);

        /**
         * Finds the value associated with a given key. Must return it by
         * value for correctness reasons---references may be invalid as
         * early as the function's return.
         *
         * Throws an exception in the event the key does not exist.
         *
         * @param key the key to find an associated value for
         */
        util::optional<Value> find(const Key & key) const;

        /**
         * Empties the map.
         */
        void clear();

    private:
        /**
         * Helper function to perform a non-mutating operation on the map.
         */
        template <class Functor>
        auto perform_operation(Functor && functor) const
            -> decltype(functor(nullptr));

        /**
         * Helper function to perform a mutating operation on the map.
         */
        template <class Functor>
        void perform_mutation(Functor && functor, bool copy = true);

        /**
         * The internal pointer to the representation map. This pointer
         * will be updated on every write operation, and is a shared
         * pointer so that it may be (atomically) copied by value into a
         * read operation's body, ensuring it remains for the duration of
         * the read, even if it has been replaced by a concurrent write.
         */
        std::shared_ptr<std::unordered_map<Key, Value>> map_;
};

}
}

#include "caching/maps/lock_free_map.tcc"
#endif
