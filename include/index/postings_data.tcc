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
    _p_id{p_id}
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

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::write_compressed(
        io::compressed_file_writer & writer) const
{
    // sort the counts according their SecondaryKey in increasing order (we know
    // SecondaryKey is an integral type)
    using pair_t = std::pair<SecondaryKey, uint64_t>;
    std::vector<pair_t> sorted{_counts.begin(), _counts.end()};
    std::sort(sorted.begin(), sorted.end(),
        [](const pair_t & a, const pair_t & b) {
            return a.first < b.first;
        }
    );

    writer.write(sorted[0].first);
    writer.write(sorted[0].second);

    // use gap encoding on the SecondaryKeys
    uint64_t cur_id = sorted[0].first;
    for(size_t i = 1; i < sorted.size(); ++i)
    {
        uint64_t temp_id = sorted[i].first;
        sorted[i].first = sorted[i].first - cur_id;
        cur_id = temp_id;

        writer.write(sorted[i].first);
        writer.write(sorted[i].second);
    }

    // mark end of postings_data
    writer.write(0);
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::read_compressed(
        io::compressed_file_reader & reader)
{
    _counts.clear();
    uint64_t last_id = 0;

    while(true)
    {
        uint64_t next_id = reader.next();

        // have we reached a delimiter?
        if(next_id == 0)
            break;

        // we're using gap encoding
        last_id += next_id;
        SecondaryKey key{last_id};

        _counts[key] = reader.next();
    }
}

}
}
