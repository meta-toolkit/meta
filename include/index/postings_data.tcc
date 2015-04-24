/**
 * @file postings_data.tcc
 * @author Sean Massung
 */

#include <algorithm>
#include <cstring>
#include <numeric>
#include "index/postings_data.h"
#include "io/binary.h"
#include "io/packed.h"

namespace meta
{
namespace index
{

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>::postings_data(PrimaryKey p_id)
    : p_id_{p_id}
{ /* nothing */
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::merge_with(postings_data& other)
{
    auto searcher = [](const pair_t& p, const SecondaryKey& s)
    {
        return p.first < s;
    };

    // O(n log n) now, could be O(n)

    // if the primary_key doesn't exist, add onto back
    uint64_t orig_length = counts_.size();
    for (auto& p : other.counts_)
    {
        auto it = std::lower_bound(
            counts_.begin(), counts_.begin() + orig_length, p.first, searcher);
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

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::increase_count(SecondaryKey s_id,
                                                             double amount)
{
    counts_[s_id] += amount;
}

template <class PrimaryKey, class SecondaryKey>
double postings_data<PrimaryKey, SecondaryKey>::count(SecondaryKey s_id) const
{
    return counts_.at(s_id);
}

template <class PrimaryKey, class SecondaryKey>
const std::vector<std::pair<SecondaryKey, double>>&
    postings_data<PrimaryKey, SecondaryKey>::counts() const
{
    return counts_.contents();
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::set_counts(const count_t& counts)
{
    // no sort needed: sparse_vector::contents() sorts the parameter
    counts_.contents(counts);
}

template <class PrimaryKey, class SecondaryKey>
template <class InputIterator>
void postings_data<PrimaryKey, SecondaryKey>::set_counts(InputIterator begin,
                                                         InputIterator end)
{
    for (; begin != end; ++begin)
        counts_.emplace_back(*begin);
    counts_.shrink_to_fit();
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::set_primary_key(
    PrimaryKey new_key)
{
    p_id_ = new_key;
}

template <class PrimaryKey, class SecondaryKey>
bool postings_data<PrimaryKey, SecondaryKey>::
    operator<(const postings_data& other) const
{
    return primary_key() < other.primary_key();
}

template <class PrimaryKey, class SecondaryKey>
bool operator==(const postings_data<PrimaryKey, SecondaryKey>& lhs,
                const postings_data<PrimaryKey, SecondaryKey>& rhs)
{
    return lhs.primary_key() == rhs.primary_key();
}

template <class PrimaryKey, class SecondaryKey>
PrimaryKey postings_data<PrimaryKey, SecondaryKey>::primary_key() const
{
    return p_id_;
}

template <class PrimaryKey, class SecondaryKey>
template <class FeatureValue>
uint64_t postings_data<PrimaryKey, SecondaryKey>::write_packed(
    std::ostream& out) const
{
    uint64_t bytes = 0;

    bytes += io::write_binary(out, p_id_);
    bytes += write_packed_counts<FeatureValue>(out);

    return bytes;
}

template <class PrimaryKey, class SecondaryKey>
template <class FeatureValue>
uint64_t postings_data<PrimaryKey, SecondaryKey>::write_packed_counts(std::ostream& out) const
{
    auto bytes = io::packed::write(out, counts_.size());

    auto total_counts
        = std::accumulate(counts_.begin(), counts_.end(), uint64_t{0},
                          [](uint64_t cur, const pair_t& pr)
                          {
                              return cur + static_cast<uint64_t>(pr.second);
                          });
    bytes += io::packed::write(out, total_counts);

    uint64_t last_id = 0;
    for (const auto& count : counts_)
    {
        bytes += io::packed::write(out, count.first - last_id);

        if (std::is_same<FeatureValue, uint64_t>::value)
        {
            bytes
                += io::packed::write(out, static_cast<uint64_t>(count.second));
        }
        else
        {
            bytes += io::packed::write(out, count.second);
        }

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
    return elem.size();
}

template <class T>
uint64_t length(const T& elem,
                typename std::enable_if<!std::is_same<T, std::string>::value>::
                    type* = nullptr)
{
    return sizeof(elem);
}
}

template <class PrimaryKey, class SecondaryKey>
template <class FeatureValue>
uint64_t postings_data<PrimaryKey, SecondaryKey>::read_packed(std::istream& in)
{
    if (in.get() == EOF)
        return 0;
    else
        in.unget();

    io::read_binary(in, p_id_);
    auto bytes = length(p_id_);

    uint64_t size;
    uint64_t total_counts;

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

        double count;
        if (std::is_same<FeatureValue, uint64_t>::value)
        {
            uint64_t next;
            bytes += io::packed::read(in, next);
            count = static_cast<double>(next);
        }
        else
        {
            bytes += io::packed::read(in, count);
        }

        counts_.emplace_back(id, count);
    }

    return bytes;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t postings_data<PrimaryKey, SecondaryKey>::bytes_used() const
{
    return sizeof(pair_t) * counts_.size() + length(p_id_);
}
}
}
