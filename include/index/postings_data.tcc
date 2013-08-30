/**
 * @file postings_data.cpp
 * @author Sean Massung
 */

#include "index/postings_data.h"
#include "util/common.h"

namespace meta {
namespace index {

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>::postings_data(PrimaryKey p_id):
    _p_id(p_id)
{ /* nothing */ }

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>::postings_data(
        const std::string & raw_data)
{
    std::istringstream iss{raw_data};
    iss >> *this;
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::merge_with(
        const postings_data & other)
{
    for(auto & p: other._counts)
        _counts[p.first] += p.second;
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::increase_count(
        SecondaryKey s_id, uint64_t amount)
{
    _counts[s_id] += amount;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t postings_data<PrimaryKey, SecondaryKey>::count(SecondaryKey s_id) const
{
    return common::safe_at(_counts, s_id);
}

template <class PrimaryKey, class SecondaryKey>
const std::unordered_map<SecondaryKey, uint64_t> &
postings_data<PrimaryKey, SecondaryKey>::counts() const
{
    return _counts;
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::set_counts(
        const std::unordered_map<SecondaryKey, uint64_t> & map)
{
    _counts = map;
}

template <class PrimaryKey, class SecondaryKey>
bool postings_data<PrimaryKey, SecondaryKey>::operator<(const postings_data & other) const
{
    return primary_key() < other.primary_key();
}

template <class PrimaryKey, class SecondaryKey>
PrimaryKey postings_data<PrimaryKey, SecondaryKey>::primary_key() const
{
    return _p_id;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t postings_data<PrimaryKey, SecondaryKey>::inverse_frequency() const
{
    return _counts.size();
}

}
}
