/**
 * @file postings_data.tcc
 * @author Sean Massung
 */

#include <algorithm>
#include <cstring>
#include <numeric>
#include "meta/index/postings_data.h"
#include "meta/io/packed.h"

namespace meta
{
namespace index
{

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
postings_data<PrimaryKey, SecondaryKey, FeatureValue>::postings_data(
    PrimaryKey p_id)
    : p_id_{p_id}
{
    // nothing
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
template <class Container>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::merge_with(
    Container&& cont)
{
    auto searcher = [](const pair_t& p, const SecondaryKey& s)
    {
        return p.first < s;
    };

    // O(n log n) now, could be O(n)

    // if the primary_key doesn't exist, add onto back
    using diff_type = typename decltype(counts_.begin())::difference_type;
    auto orig_length = counts_.size();
    for (auto& p : cont)
    {
        auto it = std::lower_bound(counts_.begin(),
                                   counts_.begin()
                                       + static_cast<diff_type>(orig_length),
                                   p.first, searcher);
        if (it == counts_.end() || it->first != p.first)
            counts_.emplace_back(std::move(p));
        else
            it->second += p.second;
    }

    // sort counts_ again to fix new elements added onto back
    if (counts_.size() > orig_length)
    {
        std::sort(counts_.begin(), counts_.end(),
                  [](const pair_t& a, const pair_t& b)
                  {
                      return a.first < b.first;
                  });
    }
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::increase_count(
    SecondaryKey s_id, FeatureValue amount)
{
    counts_[s_id] += amount;
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
FeatureValue postings_data<PrimaryKey, SecondaryKey, FeatureValue>::count(
    SecondaryKey s_id) const
{
    return counts_.at(s_id);
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
auto postings_data<PrimaryKey, SecondaryKey, FeatureValue>::counts() const
    -> const count_t &
{
    return counts_.contents();
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::set_counts(
    const count_t& counts)
{
    // no sort needed: sparse_vector::contents() sorts the parameter
    counts_.contents(counts);
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::set_counts(
    count_t&& counts)
{
    // no sort needed: sparse_vector::contents() sorts the parameter
    counts_.contents(std::move(counts));
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
template <class InputIterator>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::set_counts(
    InputIterator begin, InputIterator end)
{
    for (; begin != end; ++begin)
        counts_.emplace_back(*begin);
    counts_.shrink_to_fit();
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
void postings_data<PrimaryKey, SecondaryKey, FeatureValue>::set_primary_key(
    PrimaryKey new_key)
{
    p_id_ = new_key;
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
bool postings_data<PrimaryKey, SecondaryKey, FeatureValue>::
operator<(const postings_data& other) const
{
    return primary_key() < other.primary_key();
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
bool
operator==(const postings_data<PrimaryKey, SecondaryKey, FeatureValue>& lhs,
           const postings_data<PrimaryKey, SecondaryKey, FeatureValue>& rhs)
{
    return lhs.primary_key() == rhs.primary_key();
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
const PrimaryKey&
postings_data<PrimaryKey, SecondaryKey, FeatureValue>::primary_key() const
{
    return p_id_;
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
uint64_t postings_data<PrimaryKey, SecondaryKey, FeatureValue>::write_packed(
    std::ostream& out) const
{
    uint64_t bytes = 0;

    bytes += io::packed::write(out, p_id_);
    bytes += write_packed_counts(out);

    return bytes;
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
uint64_t
postings_data<PrimaryKey, SecondaryKey, FeatureValue>::write_packed_counts(
    std::ostream& out) const
{
    auto bytes = io::packed::write(out, counts_.size());

    auto total_counts
        = std::accumulate(counts_.begin(), counts_.end(), FeatureValue{0},
                          [](FeatureValue cur, const pair_t& pr)
                          {
                              return cur + pr.second;
                          });
    bytes += io::packed::write(out, total_counts);

    uint64_t last_id = 0;
    for (const auto& count : counts_)
    {
        bytes += io::packed::write(out, count.first - last_id);
        bytes += io::packed::write(out, count.second);
        last_id = count.first;
    }

    return bytes;
}

namespace
{
template <class T>
uint64_t length(const T& elem,
                typename std::enable_if<std::is_same<T, std::string>::value>::
                    type* = nullptr)
{
    return elem.capacity();
}

template <class T>
uint64_t length(const T& elem,
                typename std::enable_if<!std::is_same<T, std::string>::value>::
                    type* = nullptr)
{
    return sizeof(elem);
}
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
uint64_t postings_data<PrimaryKey, SecondaryKey, FeatureValue>::read_packed(
    std::istream& in)
{
    if (in.get() == EOF)
        return 0;
    else
        in.unget();

    auto bytes = io::packed::read(in, p_id_);

    uint64_t size;
    FeatureValue total_counts;

    bytes += io::packed::read(in, size);
    bytes += io::packed::read(in, total_counts);

    counts_.clear();
    counts_.reserve(size);

    SecondaryKey id{0};
    for (uint64_t i = 0; i < size; ++i)
    {
        // gap encoding
        uint64_t gap;
        bytes += io::packed::read(in, gap);
        id += gap;

        FeatureValue count;
        bytes += io::packed::read(in, count);
        counts_.emplace_back(id, count);
    }

    return bytes;
}

template <class PrimaryKey, class SecondaryKey, class FeatureValue>
uint64_t
postings_data<PrimaryKey, SecondaryKey, FeatureValue>::bytes_used() const
{
    return sizeof(pair_t) * counts_.capacity() + length(p_id_)
           + sizeof(count_t);
}
}
}
