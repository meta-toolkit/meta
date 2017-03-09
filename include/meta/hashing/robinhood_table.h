/**
 * @file robinhood_table.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_HASHING_ROBINHOOD_TABLE_H_
#define META_HASHING_ROBINHOOD_TABLE_H_

#include "meta/config.h"
#include "meta/hashing/hash.h"
#include "meta/util/aligned_allocator.h"

namespace meta
{
namespace hashing
{
namespace detail
{
template <class ValueType>
struct key_getter
{
    using key_type = ValueType;

    const ValueType& operator()(const ValueType& vt) const
    {
        return vt;
    }
};

template <class Key, class Value>
struct key_getter<std::pair<Key, Value>>
{
    using key_type = Key;

    const Key& operator()(const std::pair<Key, Value>& pr) const
    {
        return pr.first;
    }
};

template <std::size_t Size>
struct next_power_of_2;

template <>
struct next_power_of_2<sizeof(uint32_t)>
{
    uint32_t operator()(uint32_t i) const
    {
        --i;
        i |= i >> 1;
        i |= i >> 2;
        i |= i >> 4;
        i |= i >> 8;
        i |= i >> 16;
        return i + 1;
    }
};

template <>
struct next_power_of_2<sizeof(uint64_t)>
{
    uint64_t operator()(uint64_t i) const
    {
        --i;
        i |= i >> 1;
        i |= i >> 2;
        i |= i >> 4;
        i |= i >> 8;
        i |= i >> 16;
        i |= i >> 32;
        return i + 1;
    }
};

template <class ValueType, class Hash, class KeyEqual, class ValueStorage>
class robinhood_table
{
  public:
    using size_type = typename ValueStorage::size_type;
    using iterator = typename ValueStorage::iterator;
    using const_iterator = typename ValueStorage::const_iterator;
    using difference_type = typename iterator::difference_type;
    using value_type = ValueType;
    using key_getter = detail::key_getter<ValueType>;
    using key_equal = KeyEqual;
    using hasher = Hash;
    using key_type = typename key_getter::key_type;

    robinhood_table() : buckets_(8)
    {
        // nothing
    }

    iterator begin()
    {
        return entries_.begin();
    }

    const_iterator begin() const
    {
        return entries_.begin();
    }

    const_iterator cbegin() const
    {
        return entries_.begin();
    }

    iterator end()
    {
        return entries_.end();
    }

    const_iterator end() const
    {
        return entries_.end();
    }

    const_iterator cend() const
    {
        return entries_.end();
    }

    bool empty() const
    {
        return entries_.empty();
    }

    size_type size() const
    {
        return entries_.size();
    }

    constexpr size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() - 1;
    }

    void clear()
    {
        std::fill(buckets_.begin(), buckets_.end(), bucket_type{});
        entries_.clear();
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return insert(value_type{value});
    }

    template <class P>
    typename std::enable_if<std::is_constructible<value_type, P&&>::value,
                            std::pair<iterator, bool>>::type
    insert(P&& value)
    {
        value_type to_insert(std::forward<P>(value));

        return insert(std::move(to_insert));
    }

    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return insert(value_type(std::forward<Args>(args)...));
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        rehash_if_needed(next_load_factor());

        const auto hc = hasher{}(key_getter{}(value));
        const auto mask = buckets_.size() - 1;
        auto idx = hc & mask;

        std::size_t num_probes = 0;
        while (true)
        {
            if (!buckets_[idx].occupied())
            {
                entries_.push_back(std::move(value));

                buckets_[idx].hc = hc;
                buckets_[idx].idx = entries_.size();

                return std::make_pair(
                    entries_.begin()
                        + static_cast<difference_type>(buckets_[idx].eidx()),
                    true);
            }
            else if (key_equal{}(key_getter{}(value),
                                 key_getter{}(entries_[buckets_[idx].eidx()])))
            {
                return std::make_pair(
                    entries_.begin()
                        + static_cast<difference_type>(buckets_[idx].eidx()),
                    false);
            }

            const auto dib = distance_from_initial(idx);
            if (num_probes > dib)
            {
                // time to steal from the rich: the value to be inserted
                // takes the place of the current element, which is then
                // treated as if it was being inserted
                entries_.push_back(std::move(value));
                bucket_type old = buckets_[idx];

                buckets_[idx].hc = hc;
                buckets_[idx].idx = entries_.size();

                robinhood_insert(old, (idx + 1) & mask, dib + 1);

                return std::make_pair(
                    entries_.begin()
                        + static_cast<difference_type>(buckets_[idx].eidx()),
                    true);
            }

            idx = (idx + 1) & mask;
            ++num_probes;
        }
    }

    iterator erase(const_iterator pos)
    {
        auto idx = get_idx(key_getter{}(pos));
        const auto eidx = buckets_[idx].eidx();

        erase_bucket(idx);

        if (eidx > entries_.size())
            return entries_.end();

        return entries_.begin() + eidx;
    }

    size_type erase(const key_type& key)
    {
        auto idx = get_idx(key);
        if (!buckets_[idx].occupied())
            return 0;

        erase_bucket(idx);
        return 1;
    }

    void swap(robinhood_table& other)
    {
        using std::swap;
        swap(other.max_load_factor_, max_load_factor_);
        swap(other.buckets_, buckets_);
        swap(other.entries_, entries_);
    }

    iterator find(const key_type& key)
    {
        const auto idx = get_idx(key);
        if (idx >= buckets_.size() || !buckets_[idx].occupied())
            return entries_.end();
        return entries_.begin()
               + static_cast<difference_type>(buckets_[idx].eidx());
    }

    const_iterator find(const key_type& key) const
    {
        const auto idx = get_idx(key);
        if (idx >= buckets_.size() || !buckets_[idx].occupied())
            return entries_.end();
        return entries_.begin()
               + static_cast<difference_type>(buckets_[idx].eidx());
    }

    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        auto it = find(key);
        if (it == entries_.end())
            return std::make_pair(it, it);
        return std::make_pair(it, it + 1);
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& key) const
    {
        auto it = find(key);
        if (it == entries_.end())
            return std::make_pair(it, it);
        return std::make_pair(it, it + 1);
    }

    size_type count(const key_type& key) const
    {
        return find(key) != end();
    }

    constexpr static double default_max_load_factor()
    {
        return 0.95;
    }

    double load_factor() const
    {
        return static_cast<double>(size()) / buckets_.size();
    }

    double next_load_factor() const
    {
        return static_cast<double>(size() + 1) / buckets_.size();
    }

    double max_load_factor() const
    {
        return max_load_factor_;
    }

    void max_load_factor(double mlf)
    {
        max_load_factor_ = mlf;
        rehash_if_needed(load_factor());
    }

    void rehash(size_type count)
    {
        auto next_size = next_power_of_2<sizeof(size_type)>{}(count);

        // don't rehash if (1) the bucket count won't change or (2) the new
        // load factor would be bigger than the maximum allowed load factor
        if (next_size == buckets_.size()
            || static_cast<double>(size()) / next_size > max_load_factor())
        {
            return;
        }

        util::aligned_vector<bucket_type> temp_buckets(next_size);
        std::swap(temp_buckets, buckets_);

        const auto mask = buckets_.size() - 1;
        for (const auto& b : temp_buckets)
        {
            if (!b.occupied())
                continue;

            robinhood_insert(b, b.hc & mask, 0);
        }
    }

    void reserve(size_type count)
    {
        entries_.reserve(count);
        rehash(static_cast<size_type>(std::ceil(count / max_load_factor())));
    }

    hasher hash_function() const
    {
        return hasher{};
    }

    key_equal key_eq() const
    {
        return key_equal{};
    }

    size_type bytes_used() const
    {
        return sizeof(bucket_type) * buckets_.capacity()
               + sizeof(value_type) * entries_.capacity();
    }

    size_type next_bytes_used() const
    {
        auto bucket_count = buckets_.capacity();
        if (next_load_factor() >= max_load_factor())
            bucket_count *= 2;
        auto entries_count = entries_.capacity();
        if (entries_.size() == entries_.capacity())
            entries_count *= 2;
        return sizeof(bucket_type) * bucket_count
               + sizeof(value_type) * entries_count;
    }

    ValueStorage extract()
    {
        ValueStorage res;
        res.swap(entries_);
        clear();
        return res;
    }

  private:
    struct bucket_type
    {
        typename hasher::result_type hc = 0;
        size_type idx = 0;

        bool occupied() const
        {
            return idx > 0;
        }

        size_type eidx() const
        {
            return idx - 1;
        }
    };

    void rehash_if_needed(double lf)
    {
        if (lf > max_load_factor())
            rehash(buckets_.size() * 2);
    }

    std::size_t distance_from_initial(std::size_t bucket_idx) const
    {
        const auto initial_bucket
            = buckets_[bucket_idx].hc & (buckets_.size() - 1);
        if (bucket_idx < initial_bucket)
        {
            // we must have wrapped around
            return buckets_.size() - initial_bucket + bucket_idx;
        }
        else
        {
            return bucket_idx - initial_bucket;
        }
    }

    size_type get_idx(const key_type& key) const
    {
        const auto mask = buckets_.size() - 1;
        const auto hc = hasher{}(key);
        auto idx = hc & mask;

        key_equal keq;
        std::size_t num_probes = 0;
        while (true)
        {
            if (!buckets_[idx].occupied()
                || (buckets_[idx].hc == hc
                    && keq(key, key_getter{}(entries_[buckets_[idx].eidx()]))))
                return idx;

            if (num_probes > distance_from_initial(idx))
                break;

            ++num_probes;
            idx = (idx + 1) & mask;
        }

        return std::numeric_limits<size_type>::max();
    }

    // keep bumping buckets down the probe chain until we hit an empty
    // position, swapping which bucket we are moving along the way of its
    // DIB is bigger than the current number of probes
    void robinhood_insert(bucket_type b, std::size_t idx,
                          std::size_t num_probes)
    {
        const auto mask = buckets_.size() - 1;
        while (true)
        {
            if (!buckets_[idx].occupied())
            {
                buckets_[idx] = b;
                return;
            }

            const auto dib = distance_from_initial(idx);
            if (num_probes > dib)
            {
                std::swap(b, buckets_[idx]);
                num_probes = dib;
            }

            ++num_probes;
            idx = (idx + 1) & mask;
        }
    }

    void erase_bucket(size_type idx)
    {
        const auto eidx = buckets_[idx].eidx();
        std::swap(entries_[eidx], entries_.back());
        entries_.pop_back();

        buckets_[get_idx(entries_[eidx].first)].idx = buckets_[idx].idx;

        // mark this bucket as empty
        buckets_[idx].hc = 0;
        buckets_[idx].idx = 0;

        // shift all buckets that follow idx backward one position until we
        // find an unoccupied bucket or an entry that is in the right place
        const auto mask = buckets_.size() - 1;
        auto prev_idx = idx;
        idx = (idx + 1) & mask;
        while (true)
        {
            if (!buckets_[idx].occupied() || distance_from_initial(idx) == 0)
                return;

            // move the "deleted" bucket forward one slot
            std::swap(buckets_[prev_idx], buckets_[idx]);

            prev_idx = idx;
            idx = (idx + 1) & mask;
        }
    }

    double max_load_factor_ = default_max_load_factor();
    util::aligned_vector<bucket_type> buckets_;
    ValueStorage entries_;
};
}
}
}
#endif
