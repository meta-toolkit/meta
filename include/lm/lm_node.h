/**
 * @file lm_node.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_NODE_H_
#define META_LM_NODE_H_

#include <cstdint>

namespace meta
{
namespace lm
{
/**
 * Simple struct to keep track of probabilities and backoff values that is
 * packed into a uint64_t for storage.
 */
struct lm_node
{
    lm_node() : prob{0.0f}, backoff{0.0f}
    {
    }

    lm_node(float p, float b) : prob{p}, backoff{b}
    {
    }

    lm_node(uint64_t packed)
    {
        prob = *reinterpret_cast<float*>(&packed);
        backoff = *(reinterpret_cast<float*>(&packed) + 1);
    }

    bool operator==(const lm_node& other) const
    {
        return prob == other.prob && backoff == other.backoff;
    }

    float prob;
    float backoff;
};
}
}

#endif
