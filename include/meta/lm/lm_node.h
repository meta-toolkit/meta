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
#include <cstring>

#include "meta/config.h"

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
    /**
     * Default constructor.
     */
    lm_node() : prob{0.0f}, backoff{0.0f}
    {
    }

    /**
     * Parameter constructor.
     * @param p The probability value
     * @param b The backoff value
     */
    lm_node(float p, float b) : prob{p}, backoff{b}
    {
    }

    /**
     * Constructor that takes a packed [prob][backoff] uint64_t to construct
     * this node
     * @param packed
     */
    lm_node(uint64_t packed)
    {
        char* buf = reinterpret_cast<char*>(&packed);
        std::memcpy(&prob, buf, sizeof(float));
        std::memcpy(&backoff, buf + sizeof(float), sizeof(float));
    }

    /**
     * Equality operator defined so lm_node can be used in a dictionary
     */
    bool operator==(const lm_node& other) const
    {
        return prob == other.prob && backoff == other.backoff;
    }

    /**
     * @param p The probability value
     * @param b The backoff value
     * @return a packed uint64_t containing [prob][backoff]
     */
    static uint64_t write_packed(float p, float b)
    {
        uint64_t packed;
        char* buf = reinterpret_cast<char*>(&packed);
        std::memcpy(buf, &p, sizeof(float));
        std::memcpy(buf + sizeof(float), &b, sizeof(float));
        return packed;
    }

    float prob;
    float backoff;
};
}
}

#endif
