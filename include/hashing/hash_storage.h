/**
 * @file hash_storage.h
 * @author Chase Geigle
 */

#include <cassert>
#include <cmath>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "util/optional.h"

#ifndef META_HASHING_HASH_STORAGE_H_
#define META_HASHING_HASH_STORAGE_H_

namespace meta
{
namespace hashing
{

template <class T>
struct key_traits;

/**
 * Pair class used by the hash tables. This can be implicitly converted to
 * a std::pair, but is itself a lightweight wrapper around references to
 * the key and the value, which are not required to be adjacent in memory.
 */
template <class K, class V>
class kv_pair
{
  public:
    kv_pair(const K& key, V& value) : key_{key}, value_{value}
    {
        // nothing
    }

    const K& key() const
    {
        return key_.get();
    }

    const V& value() const
    {
        return value_.get();
    }

    V& value()
    {
        return value_.get();
    }

    template <class K1, class V1>
    operator std::pair<K1, V1>() const
    {
        return {key(), value()};
    }

  private:
    std::reference_wrapper<const K> key_;
    std::reference_wrapper<V> value_;
};

template <class Key>
const Key& get_key(const Key& key)
{
    return key;
}

template <class K, class V>
const K& get_key(const kv_pair<K, V>& kv)
{
    return kv.key();
}

template <class Storage>
class key_storage_iterator
{
  public:
    constexpr static bool is_const = std::is_const<Storage>::value;

    friend key_storage_iterator<typename std::add_const<Storage>::type>;

    using storage_type = Storage;

    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    using reference = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    using value_type = typename std::remove_reference<reference>::type;
    using const_value_type = typename std::add_const<value_type>::type;

    using pointer = typename std::add_pointer<value_type>::type;
    using const_pointer = typename std::add_pointer<const_value_type>::type;

    key_storage_iterator(Storage& stor) : table_{stor}, idx_{stor.capacity()}
    {
        // nothing
    }

    key_storage_iterator(Storage& stor, std::size_t idx)
        : table_{stor}, idx_{idx}
    {
        if (idx < table_.get().capacity() && !table_.get().occupied(idx))
            ++(*this);
    }

    template <class Iterator,
              class U = typename std::
                  enable_if<is_const
                            && std::is_same<
                                   typename Iterator::storage_type,
                                   typename std::remove_const<Storage>::type>::
                                   value>::type>
    key_storage_iterator(Iterator&& it)
        : table_{it.table_}, idx_{it.idx_}
    {
        // nothing
    }

    /**
     * @return the current iterator
     */
    key_storage_iterator& operator++()
    {
        do
        {
            ++idx_;
        } while (idx_ < table_.get().capacity()
                 && !table_.get().occupied(idx_));

        return *this;
    }

    /**
     * @return the key pointed to by this iterator
     */
    const_reference operator*() const
    {
        return (table_.get())[idx_];
    }

    /**
     * @return a pointer to the key pointed to by this iterator
     */
    const_pointer operator->() const
    {
        return &(table_.get())[idx_];
    }

    /**
     * @param rhs The other iterator
     * @return whether this iterator is equal to the other
     */
    bool operator==(const key_storage_iterator& rhs)
    {
        return &(table_.get()) == &(rhs.table_.get()) && idx_ == rhs.idx_;
    }

    /**
     * @param rhs The other iterator
     * @return whether this iterator is not equal to the other
     */
    bool operator!=(const key_storage_iterator& rhs)
    {
        return !(*this == rhs);
    }

  private:
    /// The probe_set this iterator is iterating over
    std::reference_wrapper<Storage> table_;

    /// The current index into the table
    std::size_t idx_;
};

template <class Storage>
class key_value_storage_iterator
{
  public:
    constexpr static bool is_const = std::is_const<Storage>::value;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using value_type =
        typename std::conditional<is_const, typename Storage::const_value_type,
                                  typename Storage::value_type>::type;
    using const_value_type = typename std::add_const<value_type>::type;
    using reference = typename std::add_lvalue_reference<value_type>::type;
    using const_reference =
        typename std::add_lvalue_reference<const_value_type>::type;
    using pointer = typename std::add_pointer<value_type>::type;
    using const_pointer = typename std::add_pointer<const_value_type>::type;

    key_value_storage_iterator(Storage& stor)
        : table_{stor}, idx_{stor.capacity()}
    {
        // nothing
    }

    key_value_storage_iterator(Storage& stor, std::size_t idx)
        : table_{stor}, idx_{idx}
    {
        if (idx < table_.get().capacity())
        {
            if (!table_.get().occupied(idx))
                ++(*this);
            else
                pair_ = table_.get()[idx];
        }
    }

    key_value_storage_iterator(const key_value_storage_iterator& other)
        : table_{other.table_}, idx_{other.idx_}, pair_{other.pair_}
    {
        // nothing
    }

    key_value_storage_iterator& operator=(const key_value_storage_iterator& rhs)
    {
        table_ = rhs.table_;
        idx_ = rhs.idx_;
        pair_ = rhs.pair_;
        return *this;
    }

    /**
     * @return the current iterator
     */
    key_value_storage_iterator& operator++()
    {
        do
        {
            ++idx_;
        } while (idx_ < table_.get().capacity()
                 && !table_.get().occupied(idx_));

        if (idx_ < table_.get().capacity() && table_.get().occupied(idx_))
            pair_ = table_.get()[idx_];

        return *this;
    }

    /**
     * @return the pair pointed to by this iterator
     */
    const_reference operator*() const
    {
        assert(pair_);
        return *pair_;
    }

    /**
     * @return the pair pointed to by this iterator
     */
    reference operator*()
    {
        assert(pair_);
        return *pair_;
    }

    /**
     * @return a proxy that acts like a pointer to the conceptual pair
     * stored in the table
     */
    const_pointer operator->() const
    {
        return &*pair_;
    }

    /**
     * @return a proxy that acts like a pointer to the conceptual pair
     * stored in the table
     */
    pointer operator->()
    {
        return &*pair_;
    }

    /**
     * @param rhs The other iterator
     * @return whether this iterator is equal to the other
     */
    bool operator==(const key_value_storage_iterator& rhs)
    {
        return &(table_.get()) == &(rhs.table_.get()) && idx_ == rhs.idx_;
    }

    /**
     * @param rhs The other iterator
     * @return whether this iterator is not equal to the other
     */
    bool operator!=(const key_value_storage_iterator& rhs)
    {
        return !(*this == rhs);
    }

  private:
    /// The probe_set this iterator is iterating over
    std::reference_wrapper<Storage> table_;

    /// The current index into the table
    std::size_t idx_;

    /// The buffered pair
    util::optional<value_type> pair_;
};

template <class Storage>
struct storage_traits;

template <class Derived>
class storage_base
{
  public:
    using iterator = typename storage_traits<Derived>::iterator;
    using const_iterator = typename storage_traits<Derived>::const_iterator;
    using key_type = typename storage_traits<Derived>::key_type;
    using stored_type = typename storage_traits<Derived>::stored_type;
    using probing_strategy = typename storage_traits<Derived>::probing_strategy;
    using hash_type = typename storage_traits<Derived>::hash_type;
    using equal_type = typename storage_traits<Derived>::equal_type;

    iterator begin()
    {
        return {as_derived(), 0};
    }

    iterator end()
    {
        return {as_derived()};
    }

    const_iterator begin() const
    {
        return {as_derived(), 0};
    }

    const_iterator end() const
    {
        return {as_derived()};
    }

    /**
     * @return the maximum allowed load factor for this table
     */
    double max_load_factor() const
    {
        return max_load_factor_;
    }

    /**
     * Sets the maximum allowed load factor for this table.
     * @param mlf The desired maximum load factor in the range (0, 1)
     */
    void max_load_factor(double mlf)
    {
        max_load_factor_ = mlf;
    }

    /**
     * @return the ratio to grow the table by when resizing
     */
    double resize_ratio() const
    {
        return resize_ratio_;
    }

    /**
     * @param rratio The desired resizing ratio to grow the table by when
     * resizing
     */
    void resize_ratio(double rratio)
    {
        resize_ratio_ = rratio;
    }

    /**
     * Uses the specified ProbingStrategy to find the specified element
     * (or the next open slot).
     *
     * @param key The key to look for
     */
    uint64_t get_idx(const key_type& key) const
    {
        probing_strategy strategy{hash_(key), as_derived().capacity()};
        auto idx = strategy.probe();
        while (as_derived().occupied(idx)
               && !equal_(get_key(as_derived()[idx]), key))
        {
            idx = strategy.probe();
        }
        return idx;
    }

    /**
     * @param args The arguments to use to construct the stored type
     * @return the index where the new item was inserted
     */
    template <class... Args>
    iterator emplace(Args&&... args)
    {
        if (next_load_factor() >= max_load_factor())
            as_derived().resize(next_size());

        stored_type stored(std::forward<Args>(args)...);
        auto idx = get_idx(storage_traits<Derived>::get_key(stored));
        as_derived().put(idx, std::move(stored));

        return {as_derived(), idx};
    }

    /**
     * @param key The key to locate in the storage
     * @return an iterator to the key, if it exists, or to the end if it
     * doesn't
     */
    const_iterator find(const key_type& key) const
    {
        auto idx = get_idx(key);

        if (!as_derived().occupied(idx))
            return end();

        return {as_derived(), idx};
    }

    /**
     * @param key The key to locate in the storage
     * @return an iterator to the key, if it exists, or to the end if it
     * doesn't
     */
    iterator find(const key_type& key)
    {
        auto idx = get_idx(key);

        if (!as_derived().occupied(idx))
            return end();

        return {as_derived(), idx};
    }

    /**
     * @return whether the table is empty
     */
    bool empty() const
    {
        return as_derived().size() == 0;
    }

    double next_load_factor() const
    {
        return (as_derived().size() + 1)
               / static_cast<double>(as_derived().capacity());
    }

    std::size_t next_size() const
    {
        return static_cast<std::size_t>(
            std::ceil(as_derived().capacity() * resize_ratio()));
    }

    bool equal(const key_type& k1, const key_type& k2) const
    {
        return equal_(k1, k2);
    }

  private:
    Derived& as_derived()
    {
        return static_cast<Derived&>(*this);
    }

    const Derived& as_derived() const
    {
        return static_cast<const Derived&>(*this);
    }

    hash_type hash_;
    equal_type equal_;
    double max_load_factor_ = 0.9;
    double resize_ratio_ = 1.5;
};

template <class T, class ProbingStrategy, class Hash, class KeyEqual>
class external_key_storage
    : public storage_base<external_key_storage<T, ProbingStrategy, Hash,
                                               KeyEqual>>
{
  public:
    using reference = T&;
    using const_reference = const T&;

    external_key_storage(std::size_t capacity) : table_(capacity, 0)
    {
        // nothing
    }

    bool occupied(std::size_t idx) const
    {
        return table_[idx] != 0;
    }

    const_reference operator[](std::size_t idx) const
    {
        return keys_[table_[idx] - 1];
    }

    reference operator[](std::size_t idx)
    {
        return keys_[table_[idx] - 1];
    }

    template <class... Args>
    void put(std::size_t idx, Args&&... args)
    {
        if (occupied(idx))
        {
            keys_[table_[idx] - 1] = T(std::forward<Args>(args)...);
        }
        else
        {
            table_[idx] = keys_.size() + 1;
            keys_.emplace_back(std::forward<Args>(args)...);
        }
    }

    std::size_t size() const
    {
        return keys_.size();
    }

    std::size_t capacity() const
    {
        return table_.size();
    }

    void clear()
    {
        std::vector<T>{}.swap(keys_);
        std::fill(std::begin(table_), std::end(table_), 0);
    }

    void resize(std::size_t new_cap)
    {
        assert(new_cap > capacity());

        table_.resize(new_cap);
        std::fill(std::begin(table_), std::end(table_), 0);

        for (std::size_t i = 0; i < keys_.size(); ++i)
            table_[this->get_idx(keys_[i])] = i + 1;
    }

    std::size_t bytes_used() const
    {
        return sizeof(std::size_t) * table_.capacity()
               + sizeof(T) * keys_.capacity();
    }

    std::vector<T> extract_keys()
    {
        auto res = std::move(keys_);
        clear();
        return res;
    }

    std::vector<std::size_t> table_;
    std::vector<T> keys_;
};

template <class T, class ProbingStrategy, class Hash, class KeyEqual>
struct storage_traits<external_key_storage<T, ProbingStrategy, Hash, KeyEqual>>
{
    using type = external_key_storage<T, ProbingStrategy, Hash, KeyEqual>;
    using iterator = key_storage_iterator<type>;
    using const_iterator = key_storage_iterator<const type>;
    using stored_type = T;
    using key_type = T;
    using probing_strategy = ProbingStrategy;
    using hash_type = Hash;
    using equal_type = KeyEqual;

    static const T& get_key(const T& key)
    {
        return key;
    }
};

template <class T, class ProbingStrategy, class Hash, class KeyEqual>
class inline_key_storage
    : public storage_base<inline_key_storage<T, ProbingStrategy, Hash,
                                             KeyEqual>>
{
  public:
    using reference = T&;
    using const_reference = const T&;

    inline_key_storage(std::size_t capacity)
        : table_(capacity, key_traits<T>::sentinel()), size_{0}
    {
        // nothing
    }

    bool occupied(std::size_t idx) const
    {
        return !this->equal(table_[idx], key_traits<T>::sentinel());
    }

    const_reference operator[](std::size_t idx) const
    {
        return table_[idx];
    }

    reference operator[](std::size_t idx)
    {
        return table_[idx];
    }

    template <class... Args>
    void put(std::size_t idx, Args&&... args)
    {
        if (!occupied(idx))
            ++size_;
        table_[idx] = T(std::forward<Args>(args)...);
    }

    std::size_t size() const
    {
        return size_;
    }

    std::size_t capacity() const
    {
        return table_.size();
    }

    void clear()
    {
        std::fill(std::begin(table_), std::end(table_),
                  key_traits<T>::sentinel());
        size_ = 0;
    }

    void resize(std::size_t new_cap)
    {
        assert(new_cap > capacity());

        std::vector<T> temptable(new_cap, key_traits<T>::sentinel());
        using std::swap;
        swap(table_, temptable);

        for (std::size_t idx = 0; idx < temptable.size(); ++idx)
        {
            if (!this->equal(temptable[idx], key_traits<T>::sentinel()))
                table_[this->get_idx(temptable[idx])]
                    = std::move(temptable[idx]);
        }
    }

    std::size_t bytes_used() const
    {
        return sizeof(T) * table_.capacity() + sizeof(std::size_t);
    }

    std::vector<T> extract_keys()
    {
        std::vector<T> res;
        res.reserve(size_);
        for (std::size_t idx = 0; idx < table_.size(); ++idx)
        {
            if (occupied(idx))
                res.emplace_back(std::move(table_[idx]));
        }
        clear();
        return res;
    }

    std::vector<T> table_;
    std::size_t size_;
};

template <class T, class ProbingStrategy, class Hash, class KeyEqual>
struct storage_traits<inline_key_storage<T, ProbingStrategy, Hash, KeyEqual>>
{
    using type = inline_key_storage<T, ProbingStrategy, Hash, KeyEqual>;
    using iterator = key_storage_iterator<type>;
    using const_iterator = key_storage_iterator<const type>;
    using stored_type = T;
    using key_type = T;
    using probing_strategy = ProbingStrategy;
    using hash_type = Hash;
    using equal_type = KeyEqual;

    static const T& get_key(const T& key)
    {
        return key;
    }
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
class inline_key_value_storage
    : public storage_base<inline_key_value_storage<K, V, ProbingStrategy, Hash,
                                                   KeyEqual>>
{
  public:
    using value_type = kv_pair<K, V>;
    using const_value_type = kv_pair<K, const V>;

    inline_key_value_storage(std::size_t capacity)
        : table_(capacity, std::make_pair(key_traits<K>::sentinel(),
                                          key_traits<V>::sentinel())),
          size_{0}
    {
        // nothing
    }

    bool occupied(std::size_t idx) const
    {
        return !this->equal(table_[idx].first, key_traits<K>::sentinel());
    }

    const_value_type operator[](std::size_t idx) const
    {
        const auto& pr = table_[idx];
        return {pr.first, pr.second};
    }

    value_type operator[](std::size_t idx)
    {
        auto& pr = table_[idx];
        return {pr.first, pr.second};
    }

    template <class... Args>
    void put(std::size_t idx, Args&&... args)
    {
        if (!occupied(idx))
            ++size_;
        table_[idx] = std::pair<K, V>(std::forward<Args>(args)...);
    }

    std::size_t size() const
    {
        return size_;
    }

    std::size_t capacity() const
    {
        return table_.size();
    }

    void clear()
    {
        std::fill(std::begin(table_), std::end(table_),
                  std::make_pair(key_traits<K>::sentinel(),
                                 key_traits<V>::sentinel()));
        size_ = 0;
    }

    void resize(std::size_t new_cap)
    {
        assert(new_cap > capacity());

        std::vector<std::pair<K, V>> temptable(
            new_cap, std::make_pair(key_traits<K>::sentinel(),
                                    key_traits<V>::sentinel()));
        using std::swap;
        swap(table_, temptable);

        for (std::size_t i = 0; i < temptable.size(); ++i)
        {
            if (!this->equal(temptable[i].first, key_traits<K>::sentinel()))
                table_[this->get_idx(temptable[i].first)]
                    = std::move(temptable[i]);
        }
    }

    std::size_t bytes_used() const
    {
        return sizeof(std::pair<K, V>) * table_.capacity()
               + sizeof(std::size_t);
    }

    std::vector<std::pair<K, V>> table_;
    std::size_t size_;
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
struct storage_traits<inline_key_value_storage<K, V, ProbingStrategy, Hash,
                                               KeyEqual>>
{
    using type
        = inline_key_value_storage<K, V, ProbingStrategy, Hash, KeyEqual>;
    using iterator = key_value_storage_iterator<type>;
    using const_iterator = key_value_storage_iterator<const type>;
    using stored_type = std::pair<K, V>;
    using key_type = K;
    using probing_strategy = ProbingStrategy;
    using hash_type = Hash;
    using equal_type = KeyEqual;

    static const key_type& get_key(const stored_type& st)
    {
        return st.first;
    }
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
class inline_key_external_value_storage
    : public storage_base<inline_key_external_value_storage<K, V,
                                                            ProbingStrategy,
                                                            Hash, KeyEqual>>
{
  public:
    using value_type = kv_pair<K, V>;
    using const_value_type = kv_pair<K, const V>;

    inline_key_external_value_storage(std::size_t capacity)
        : table_(capacity,
                 std::make_pair(key_traits<K>::sentinel(), std::size_t{0}))
    {
        // nothing
    }

    bool occupied(std::size_t idx) const
    {
        return !this->equal(table_[idx].first, key_traits<K>::sentinel());
    }

    const_value_type operator[](std::size_t idx) const
    {
        const auto& pr = table_[idx];
        return {pr.first, values_[pr.second]};
    }

    value_type operator[](std::size_t idx)
    {
        auto& pr = table_[idx];
        return {pr.first, values_[pr.second]};
    }

    template <class... Args>
    void put(std::size_t idx, Args&&... args)
    {
        auto pr = std::pair<K, V>(std::forward<Args>(args)...);
        if (occupied(idx))
        {
            values_[table_[idx].second] = std::move(pr.second);
        }
        else
        {
            table_[idx].second = values_.size();
            values_.emplace_back(std::move(pr.second));
        }
        table_[idx].first = std::move(pr.first);
    }

    std::size_t size() const
    {
        return values_.size();
    }

    std::size_t capacity() const
    {
        return table_.size();
    }

    void clear()
    {
        std::fill(std::begin(table_), std::end(table_),
                  std::make_pair(key_traits<K>::sentinel(), std::size_t{0}));
        std::vector<V>{}.swap(values_);
    }

    void resize(std::size_t new_cap)
    {
        assert(new_cap > capacity());

        std::vector<std::pair<K, std::size_t>> temptable(
            new_cap, std::make_pair(key_traits<K>::sentinel(), 0));
        using std::swap;
        swap(table_, temptable);

        for (std::size_t i = 0; i < temptable.size(); ++i)
        {
            if (!this->equal(temptable[i].first, key_traits<K>::sentinel()))
                table_[this->get_idx(temptable[i].first)]
                    = std::move(temptable[i]);
        }
    }

    std::size_t bytes_used() const
    {
        return sizeof(std::pair<K, std::size_t>) * table_.capacity()
               + sizeof(V) * values_.capacity();
    }

    std::vector<std::pair<K, std::size_t>> table_;
    std::vector<V> values_;
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
struct storage_traits<inline_key_external_value_storage<K, V, ProbingStrategy,
                                                        Hash, KeyEqual>>
{
    using type = inline_key_external_value_storage<K, V, ProbingStrategy, Hash,
                                                   KeyEqual>;
    using iterator = key_value_storage_iterator<type>;
    using const_iterator = key_value_storage_iterator<const type>;
    using stored_type = std::pair<K, V>;
    using key_type = K;
    using probing_strategy = ProbingStrategy;
    using hash_type = Hash;
    using equal_type = KeyEqual;

    static const key_type& get_key(const stored_type& st)
    {
        return st.first;
    }
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
class external_key_value_storage
    : public storage_base<external_key_value_storage<K, V, ProbingStrategy,
                                                     Hash, KeyEqual>>
{
  public:
    using value_type = kv_pair<K, V>;
    using const_value_type = kv_pair<K, const V>;

    external_key_value_storage(std::size_t capacity) : table_(capacity, 0)
    {
        // nothing
    }

    bool occupied(std::size_t idx) const
    {
        return table_[idx] != 0;
    }

    const_value_type operator[](std::size_t idx) const
    {
        const auto& pr = storage_[table_[idx] - 1];
        return {pr.first, pr.second};
    }

    value_type operator[](std::size_t idx)
    {
        auto& pr = storage_[table_[idx] - 1];
        return {pr.first, pr.second};
    }

    template <class... Args>
    void put(std::size_t idx, Args&&... args)
    {
        if (occupied(idx))
        {
            storage_[table_[idx] - 1]
                = std::pair<K, V>(std::forward<Args>(args)...);
        }
        else
        {
            table_[idx] = storage_.size() + 1;
            storage_.emplace_back(std::forward<Args>(args)...);
        }
    }

    std::size_t size() const
    {
        return storage_.size();
    }

    std::size_t capacity() const
    {
        return table_.size();
    }

    void clear()
    {
        std::vector<std::pair<K, V>>{}.swap(storage_);
        std::fill(std::begin(table_), std::end(table_), 0);
    }

    void resize(std::size_t new_cap)
    {
        assert(new_cap > capacity());

        table_.resize(new_cap);
        std::fill(std::begin(table_), std::end(table_), 0);

        for (std::size_t i = 0; i < storage_.size(); ++i)
            table_[this->get_idx(storage_[i].first)] = i + 1;
    }

    std::size_t bytes_used() const
    {
        return sizeof(std::size_t) * table_.capacity()
               + sizeof(std::pair<K, V>) * storage_.capacity();
    }

    std::vector<std::size_t> table_;
    std::vector<std::pair<K, V>> storage_;
};

template <class K, class V, class ProbingStrategy, class Hash, class KeyEqual>
struct storage_traits<external_key_value_storage<K, V, ProbingStrategy, Hash,
                                                 KeyEqual>>
{
    using type
        = external_key_value_storage<K, V, ProbingStrategy, Hash, KeyEqual>;
    using iterator = key_value_storage_iterator<type>;
    using const_iterator = key_value_storage_iterator<const type>;
    using stored_type = std::pair<K, V>;
    using key_type = K;
    using probing_strategy = ProbingStrategy;
    using hash_type = Hash;
    using equal_type = KeyEqual;

    static const key_type& get_key(const stored_type& st)
    {
        return st.first;
    }
};
}
}
#endif
