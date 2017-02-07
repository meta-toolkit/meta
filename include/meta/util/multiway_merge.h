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

#include "meta/config.h"
#include "meta/io/filesystem.h"
#include "meta/io/mmap_file.h"
#include "meta/io/moveable_stream.h"
#include "meta/io/packed.h"
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
 *     must have a member function merge_with(Record&&). During the merging
 *     process, Records will be read from the individual chunks (via
 *     ChunkIterator::operator++), merge_with will be called across all
 *     Records across all chunks that should merge according to the
 *     predicate specified (defaulting to operator==), and the final merged
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
 * - Compare:
 *     A simple comparison function to be used for sorting the records.
 *     Defaults to operator<.
 *
 * - ShouldMerge:
 *     A binary function that returns true if the two records given to it
 *     as arguments should be merged together via Record::merge_with().
 *     Defaults to operator==.
 *
 * - RecordHandler:
 *     A unary function that is called once per every unique Record after
 *     merging.
 *
 * - ProgressTrait:
 *     A traits class whose type indicates the progress reporting object to
 *     use. By default, this is meta::printing::default_progress_trait, but
 *     progress reporting can be silenced using
 *     meta::printing::no_progress_trait.
 *
 * @return the total number of unique Records that were written to the
 * OutputStream
 */
template <class ForwardIterator, class RecordHandler, class Compare,
          class ShouldMerge,
          class ProgressTrait = printing::default_progress_trait>
uint64_t multiway_merge(ForwardIterator begin, ForwardIterator end,
                        Compare&& record_comp, ShouldMerge&& should_merge,
                        RecordHandler&& output, ProgressTrait = ProgressTrait{})
{
    using ChunkIterator = typename ForwardIterator::value_type;

    uint64_t to_read = std::accumulate(
        begin, end, 0ul, [](uint64_t acc, const ChunkIterator& chunk) {
            return acc + chunk.total_bytes();
        });

    typename ProgressTrait::type progress{" > Merging: ", to_read};

    uint64_t total_read = std::accumulate(
        begin, end, 0ul, [](uint64_t acc, const ChunkIterator& chunk) {
            return acc + chunk.bytes_read();
        });

    std::vector<std::reference_wrapper<ChunkIterator>> to_merge;
    to_merge.reserve(static_cast<std::size_t>(std::distance(begin, end)));
    for (; begin != end; ++begin)
        to_merge.emplace_back(*begin);

    auto chunk_iter_comp = [&](const ChunkIterator& a, const ChunkIterator& b) {
        return record_comp(*a, *b);
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
        auto before = (*range.first).get().bytes_read();
        ++(*range.first).get();
        total_read += ((*range.first).get().bytes_read() - before);
        ++range.first;
        std::for_each(range.first, range.second, [&](ChunkIterator& iter) {
            if (should_merge(merged, *iter))
            {
                merged.merge_with(std::move(*iter));
                auto before = iter.bytes_read();
                ++iter;
                total_read += (iter.bytes_read() - before);
            }
        });

        // write out merged record
        output(std::move(merged));

        // remove all empty ChunkIterators from the input
        to_merge.erase(std::remove_if(to_merge.begin(), to_merge.end(),
                                      [](const ChunkIterator& iter) {
                                          return iter == ChunkIterator{};
                                      }),
                       to_merge.end());
    }

    return unique_records;
}

/**
 * A simplified wrapper for multiway_merge that uses the default comparison
 * (operator<) and merge criteria (operator==).
 */
template <class ForwardIterator, class RecordHandler,
          class ProgressTrait = printing::default_progress_trait>
uint64_t multiway_merge(ForwardIterator begin, ForwardIterator end,
                        RecordHandler&& output, ProgressTrait = ProgressTrait{})
{
    using Record = typename std::remove_reference<decltype(**begin)>::type;

    auto record_comp = [](const Record& a, const Record& b) { return a < b; };
    auto record_equal = [](const Record& a, const Record& b) { return a == b; };
    return multiway_merge(begin, end, record_comp, record_equal,
                          std::forward<RecordHandler>(output), ProgressTrait{});
}

/**
 * A simple implementation of the ChunkIterator concept that reads Records
 * from a binary file using io::packed::read.
 */
template <class Record>
class chunk_iterator
{
  public:
    /// Default constructor (end iterator)
    chunk_iterator() = default;

    /**
     * Constructs a new chunk_iterator reading from the given filename
     * @param filename The file to read from
     */
    chunk_iterator(const std::string& filename)
        : input_{filename},
          bytes_read_{0},
          total_bytes_{filesystem::file_size(filename)}
    {
        ++(*this);
    }

    /**
     * Moves the chunk iterator forward in the file to the next Record. If
     * the file hits EOF, the internal stream is closed and the resulting
     * iterator will compare equal with the end iterator.
     *
     * @return the current iterator
     */
    chunk_iterator& operator++()
    {
        if (input_.peek() == EOF)
        {
            input_.close();

            assert(*this == chunk_iterator{});
            return *this;
        }

        bytes_read_ += io::packed::read(input_, record_);
        return *this;
    }

    Record& operator*()
    {
        return record_;
    }

    const Record& operator*() const
    {
        return record_;
    }

    uint64_t total_bytes() const
    {
        return total_bytes_;
    }

    uint64_t bytes_read() const
    {
        return bytes_read_;
    }

    /**
     * Equality of chunk_iterators only holds when both iterators are the
     * "end" iterator.
     *
     * @param other The other iterator to compare against
     * @return whether both iterators are the end iterator
     */
    bool operator==(const chunk_iterator& other) const
    {
        return !input_.is_open() && !other.input_.is_open();
    }

  private:
    io::mmap_ifstream input_;
    Record record_;
    uint64_t bytes_read_;
    uint64_t total_bytes_;
};

template <class Record>
bool operator!=(const chunk_iterator<Record>& a,
                const chunk_iterator<Record>& b)
{
    return !(a == b);
}

/**
 * A simple implementation of the ChunkIterator concept that reads Records
 * from a binary file using io::packed::read and deletes the underlying
 * file when it reaches EOF.
 */
template <class Record>
class destructive_chunk_iterator : public chunk_iterator<Record>
{
  public:
    using base_iterator = chunk_iterator<Record>;

    destructive_chunk_iterator() = default;

    destructive_chunk_iterator(const std::string& filename)
        : base_iterator(filename), filename_{filename}
    {
        // nothing
    }

    destructive_chunk_iterator& operator++()
    {
        ++base();
        if (base() == base_iterator{})
            filesystem::delete_file(filename_);

        return *this;
    }

    const std::string& filename() const
    {
        return filename_;
    }

  private:
    base_iterator& base()
    {
        return static_cast<base_iterator&>(*this);
    }

    const std::string filename_;
};
}
}
#endif
