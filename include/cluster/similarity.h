/**
 * @file similarity.h
 */

#ifndef _SIMILARITY_H_
#define _SIMILARITY_H_

#include <unordered_map>
#include <unordered_set>

/**
 * Contains various similarity metrics. These are intended to be used by
 * specific objects (e.g. Document) to implement their own similarity functions.
 *
 * All functions take in unordered_map<Key, Value> representing a sparse vector.
 * Value is assumed to be a numerical value that math operations can be
 * performed on.
 */
namespace Similarity
{
    using std::unordered_map;
    using std::unordered_set;

    /**
     * Computes the cosine similarity between two sparse vectors.
     * \f$ similarity(a, b) = \frac{a\cdot b}{||a||\cdot ||b||} \f$
     * @param a
     * @param b
     * @return the cosine similarity between the two parameters
     */
    template <class Key, class Value>
    double cosine_similarity(const unordered_map<Key, Value> & a,
                             const unordered_map<Key, Value> & b);

    struct cosine {
        template <class Key, class Value>
        double operator()(const unordered_map<Key, Value> & a,
                          const unordered_map<Key, Value> & b) {
            return cosine_similarity(a, b);
        }
    };


    /**
     * Computes the Jaccard similarity between two sparse vectors.
     * \f$ similarity(a, b) = \frac{|a\cap b|}{|a\cup b|} \f$
     * @param a
     * @param b
     * @return the Jaccard similarity between the two parameters
     */
    template <class Key, class Value>
    double jaccard_similarity(const unordered_map<Key, Value> & a,
                              const unordered_map<Key, Value> & b);

    struct jaccard {
        template <class Key, class Value>
        double operator()(const unordered_map<Key, Value> & a,
                          const unordered_map<Key, Value> & b) {
            return jaccard_similarity(a, b);
        }
    };

    /**
     * Contains "private" helpers for the Similarity namespace.
     */
    namespace internal
    {
        /**
         * @return the keyspace shared between the two sparse vectors.
         */
        template <class Key, class Value>
        unordered_set<Key> get_space(const unordered_map<Key, Value> & a,
                                     const unordered_map<Key, Value> & b);
        
        /**
         * @param map The sparse vector.
         * @return the magnitude of a sparse vector.
         */
        template <class Key, class Value>
        double magnitude(const unordered_map<Key, Value> & map);

        /**
         * @return the value for a given key in map if it exists; otherwise,
         * the default Value() is returned.
         */
        template <class Key, class Value>
        Value safe_at(const unordered_map<Key, Value> & map, const Key & key);
    }
}

#include "cluster/similarity.tcc"
#endif
