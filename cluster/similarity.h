/**
 * @file similarity.h
 */

#ifndef _SIMILARITY_H_
#define _SIMILARITY_H_

#include <unordered_map>

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
    /**
     * Computes the cosine similarity between two sparse vectors.
     */
    template<class Key, class Value>
    double cosine_similarity(const std::unordered_map<Key, Value> & a,
                             const std::unordered_map<Key, Value> & b);

    /**
     * Computes the Jaccard similarity between two sparse vectors.
     */
    template<class Key, class Value>
    double jaccard_similarity(const std::unordered_map<Key, Value> & a,
                             const std::unordered_map<Key, Value> & b);
}

#include "similarity.cpp"
#endif
