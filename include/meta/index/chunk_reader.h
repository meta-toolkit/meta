/**
 * @file chunk_reader.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_CHUNK_READER_H_
#define META_INDEX_CHUNK_READER_H_

#include <algorithm>
#include <fstream>
#include <memory>
#include <numeric>
#include <string>

#include "meta/config.h"
#include "meta/io/filesystem.h"
#include "meta/io/moveable_stream.h"
#include "meta/util/multiway_merge.h"
#include "meta/util/progress.h"

namespace meta
{
namespace index
{

/**
 * Simple wrapper class to adapt PostingsData to the Record concept for
 * multiway_merge.
 */
template <class PostingsData>
class postings_record
{
  public:
    using primary_key_type = typename PostingsData::primary_key_type;
    using count_t = typename PostingsData::count_t;

    postings_record() = default;

    operator PostingsData() &&
    {
        PostingsData pdata{key_};
        pdata.set_counts(std::move(counts_));
        return pdata;
    }

    void merge_with(postings_record&& other)
    {
        std::move(other.counts_.begin(), other.counts_.end(),
                  std::back_inserter(counts_));
        count_t{}.swap(other.counts_);
    }

    template <class InputStream>
    uint64_t read(InputStream& in)
    {
        PostingsData pdata;
        auto bytes = pdata.read_packed(in);
        key_ = pdata.primary_key();
        counts_ = pdata.counts();
        return bytes;
    }

    bool operator<(const postings_record& other) const
    {
        return key_ < other.key_;
    }

    bool operator==(const postings_record& other) const
    {
        return key_ == other.key_;
    }

    count_t& counts() const
    {
        return counts_;
    }

    template <class InputStream>
    friend uint64_t packed_read(InputStream& is, postings_record& record)
    {
        PostingsData pdata;
        auto bytes = pdata.read_packed(is);
        record.key_ = pdata.primary_key();
        record.counts_ = pdata.counts();
        return bytes;
    }

  private:
    primary_key_type key_;
    count_t counts_;
};

/**
 * Represents an on-disk chunk to be merged with multi-way merge sort. Each
 * chunk_reader stores the file it's reading from, the total bytes needed
 * to be read, and the current number of bytes read, as well as buffers in
 * one postings_record.
 */
template <class PostingsData>
using chunk_reader = util::chunk_iterator<postings_record<PostingsData>>;

/**
 * Performs a multi-way merge sort of all of the provided chunks, writing
 * to the provided output stream. Currently, this function will attempt
 * to open std::distance(begin, end) number of files and merge them all
 * simultaneously but this could change in future implementations.
 *
 * @param outstream Where the merged chunks should be written
 * @param begin An iterator to the beginning of the sequence containing
 *  the chunk paths
 * @param end An iterator to the end of the sequence containing the chunk
 *  paths
 * @return the total number of unique primary keys found during the merging
 */
template <class PostingsData, class ForwardIterator>
uint64_t multiway_merge(std::ostream& outstream, ForwardIterator begin,
                        ForwardIterator end)
{
    using input_chunk = chunk_reader<PostingsData>;
    std::vector<input_chunk> to_merge;
    to_merge.reserve(static_cast<std::size_t>(std::distance(begin, end)));
    for (; begin != end; ++begin)
        to_merge.emplace_back(*begin);

    return util::multiway_merge(
        to_merge.begin(), to_merge.end(),
        [&](PostingsData&& pdata) { pdata.write_packed(outstream); });
}
}
}
#endif
