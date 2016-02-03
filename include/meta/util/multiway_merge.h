/**
 * @file multiway_merge.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_UTIL_MULTIWAY_MERGE_H_
#define META_UTIL_MULTIWAY_MERGE_H_

#include <cstdint>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

#include "meta/util/progress.h"

namespace meta
{
namespace util
{

/**
 * A generic algorithm for performing an N-way merge on a collection of
 * sorted "chunks".
 *
 * The following concepts are involved:
 *
 * - Record:
 *     A Record must represent the atomic items that are to be merged. They
 *     are comparable via operator< and operator==, and must have a member
 *     function merge_with(Record&&). During the merging process, Records
 *     will be read from the individual chunks (via
 *     ChunkIterator::operator++), merge_with will be called across all
 *     Records across all chunks that compare equal, and the final merged
 *     Record will be passed to the write callback.
 *
 * - ForwardIterator:
 *     A basic ForwardIterator whose value type must model the
 *     ChunkIterator concept.
 *
 * - ChunkIterator:
 *     A ChunkIterator is an iterator over Records within a chunk.
 *     Typically, these will be streamed-in from disk, and thus
 *     ChunkIterator can be thought of as an InputIterator with a few
 *     additional functions.
 *
 *     ChunkIterators must dereference to a Record.
 *
 *     Comparison operators for (in)equality must be provided. A
 *     ChunkIterator that cannot read any more Records must compare equal
 *     to the default-constructed ChunkIterator.
 *
 *     ChunkIterator's operator++ shall buffer in (at least) the next
 *     Record, which can be obtained via the dereference operator.
 *
 *     ChunkIterators also keep track of the state of the underlying
 *     InputStream from which Records are read. ChunkIterators shall
 *     provide two observer functions for the state of that stream:
 *     total_bytes(), which indicates the total number of bytes that occur
 *     in the stream, and bytes_read(), which indicates the total number of
 *     bytes that have been consumed from the stream thus far. When
 *     bytes_read() == total_bytes(), the stream has been exhausted and the
 *     iterator shall compare equal to the default-constructed
 *     ChunkIterator.
 *
 * @return the total number of unique Records that were written to the
 * OutputStream
 */

template <class ForwardIterator, class RecordHandler>
uint64_t multiway_merge(ForwardIterator begin, ForwardIterator end,
                        RecordHandler&& output)
{
    using ChunkIterator = typename ForwardIterator::value_type;
    using Record = typename ChunkIterator::value_type;

    uint64_t to_read = std::accumulate(
        begin, end, 0ul, [](uint64_t acc, const ChunkIterator& chunk)
        {
            return acc + chunk.total_bytes();
        });

    printing::progress progress{" > Merging: ", to_read};

    uint64_t total_read = std::accumulate(
        begin, end, 0ul, [](uint64_t acc, const ChunkIterator& chunk)
        {
            return acc + chunk.bytes_read();
        });

    std::vector<std::reference_wrapper<ChunkIterator>> to_merge;
    to_merge.reserve(static_cast<std::size_t>(std::distance(begin, end)));
    for (; begin != end; ++begin)
        to_merge.emplace_back(*begin);

    auto chunk_iter_comp = [](const ChunkIterator& a, const ChunkIterator& b)
    {
        return *a < *b;
    };

    uint64_t unique_records = 0;
    while (!to_merge.empty())
    {
        progress(total_read);
        ++unique_records;

        std::sort(to_merge.begin(), to_merge.end(), chunk_iter_comp);

        // gather all Records that match the smallest Record, reading in a
        // new Record from the corresponding ChunkIterator
        auto range = std::equal_range(to_merge.begin(), to_merge.end(),
                                      to_merge.front(), chunk_iter_comp);

        auto merged = std::move(*(*range.first).get());
        ++(*range.first).get();
        ++range.first;
        std::for_each(range.first, range.second, [&](ChunkIterator& iter)
                      {
                          merged.merge_with(std::move(*iter));
                          auto before = iter.bytes_read();
                          ++iter;
                          total_read += (iter.bytes_read() - before);
                      });

        // write out merged record
        output(std::move(merged));

        // remove all empty ChunkIterators from the input
        to_merge.erase(std::remove_if(to_merge.begin(), to_merge.end(),
                                      [](const ChunkIterator& iter)
                                      {
                                          return iter == ChunkIterator{};
                                      }),
                       to_merge.end());
    }

    return unique_records;
}
}
}
#endif
