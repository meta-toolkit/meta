/**
 * @file probing.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_PROBING_H_
#define META_HASHING_PROBING_H_

#include <cstddef>
#include <cstdint>

#include "meta/config.h"
#include "meta/hashing/hash_traits.h"
#include "meta/util/likely.h"

namespace meta
{
namespace hashing
{
namespace probing
{

class linear
{
  public:
    linear(uint64_t hash, uint64_t capacity) : hash_{hash}, capacity_{capacity}
    {
        hash_ %= capacity_;
    }

    /**
     * @return the next index to probe in the table
     */
    uint64_t probe()
    {
        return hash_++ % capacity_;
    }

  private:
    uint64_t hash_;
    uint64_t capacity_;
};

class linear_nomod
{
  public:
    linear_nomod(uint64_t hash, uint64_t capacity)
        : hash_{hash}, max_{capacity - 1}
    {
        hash_ %= capacity;
    }

    /**
     * @return the next index to probe in the table
     */
    uint64_t probe()
    {
        hash_++;
        if (hash_ > max_)
            hash_ = 0;
        return hash_;
    }

  private:
    uint64_t hash_;
    uint64_t max_;
};

class binary
{
  public:
    binary(uint64_t hash, uint64_t capacity)
        : hash_{hash}, step_{0}, capacity_{capacity}
    {
        hash_ %= capacity;
    }

    /**
     * @return the next index to probe in the table
     */
    uint64_t probe()
    {
        // discard hashes that fall off of the table
        for (; (hash_ ^ step_) >= capacity_; ++step_)
            ;
        return hash_ ^ step_++;
    }

  private:
    uint64_t hash_;
    uint64_t step_;
    uint64_t capacity_;
};

template <class T, std::size_t Alignment = 64>
class binary_hybrid
{
  public:
    using probe_entry = typename hash_traits<T>::probe_entry;

    static_assert(Alignment > sizeof(probe_entry),
                  "Alignment should be larger than sizeof(T)");
    const static uint64_t block_size = Alignment / sizeof(probe_entry);

    binary_hybrid(uint64_t hash, uint64_t capacity)
        : hash_{hash}, step_{0}, max_{capacity - 1}
    {
        hash_ %= capacity;

        // find the index of the last (potentially) partial block
        auto last_block_start = capacity & ~(block_size - 1);
        if (META_UNLIKELY(hash_ >= last_block_start))
        {
            step_ = block_size;
            idx_ = hash_;
        }
        else
        {
            // idx_ is the index of the start of the next block. If this is off
            // the table the condition in probe() will fix it.
            idx_ = (hash_ | (block_size - 1)) + 1;
        }
    }

    uint64_t probe()
    {
        if (META_LIKELY(step_ < block_size))
        {
            return hash_ ^ step_++;
        }
        else
        {
            if (META_UNLIKELY(idx_ > max_))
                idx_ = 0;
            return idx_++;
        }
    }

  private:
    uint64_t hash_;
    uint64_t step_;
    uint64_t idx_;
    uint64_t max_;
};

// http://stackoverflow.com/questions/2348187
//     /moving-from-linear-probing-to-quadratic-probing-hash-collisons
class quadratic
{
  public:
    quadratic(uint64_t hash, uint64_t capacity)
        : hash_{hash}, capacity_{capacity}, step_{0}
    {
        hash_ &= (capacity_ - 1);
    }

    /**
     * @note This strategy only will work for power-of-2 capacities!
     * @return the next index to probe in the table
     */
    uint64_t probe()
    {
        auto next = (hash_ + (step_ * (step_ + 1)) / 2) & (capacity_ - 1);
        ++step_;
        return next;
    }

  private:
    uint64_t hash_;
    uint64_t capacity_;
    uint64_t step_;
};
}
}
}
#endif
