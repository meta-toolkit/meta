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
#include <numeric>
#include <memory>
#include <string>

#include "util/filesystem.h"
#include "util/progress.h"
#include "util/shim.h"

namespace meta
{
namespace index
{

/**
 * Represents an on-disk chunk to be merged with multi-way merge sort. Each
 * chunk_reader stores the file it's reading from, the total bytes needed
 * to be read, and the current number of bytes read, as well as buffers in
 * one postings.
 */
template <class PostingsData>
class chunk_reader
{
  private:
    /// the file we're reading from currently, or null if there is none
    std::unique_ptr<std::ifstream> file_;
    /// the path to the file we're reading from
    std::string path_;
    /// the current buffered postings data
    PostingsData postings_;
    /// the total number of bytes in the chunk we're reading
    uint64_t total_bytes_;
    /// the total number of bytes read
    uint64_t bytes_read_;

  public:
    /**
     * Constructs a new chunk reader from the given chunk path.
     * @param filename The path to the chunk to be read
     */
    chunk_reader(const std::string& filename)
        : file_{make_unique<std::ifstream>(filename, std::ios::binary)},
          path_{filename},
          total_bytes_{filesystem::file_size(path_)},
          bytes_read_{0}
    {
        ++(*this);
    }

    /**
     * Destroys the reader **and the chunk file it was reading from**.
     */
    ~chunk_reader()
    {
        if (file_)
        {
            file_ = nullptr;
            filesystem::delete_file(path_);
        }
    }

    /**
     * chunk_reader can be move constructed.
     */
    chunk_reader(chunk_reader&&) = default;

    /**
     * chunk_reader can be move assigned.
     * @param rhs the right hand side of the assignment
     */
    chunk_reader& operator=(chunk_reader&& rhs)
    {
        if (file_)
        {
            file_ = nullptr;
            filesystem::delete_file(path_);
        }

        file_ = std::move(rhs.file_);
        path_ = std::move(rhs.path_);
        postings_ = std::move(rhs.postings_);
        total_bytes_ = rhs.total_bytes_;
        bytes_read_ = rhs.bytes_read_;

        return *this;
    }

    /**
     * Whether or not the chunk_reader is in a good state.
     * @return whether the underlying stream is in a good state
     */
    operator bool() const
    {
        return static_cast<bool>(*file_);
    }

    /**
     * Comparison operator for sorting.
     * @param other the other reader to compare with
     * @return whether the current reader's postings's primary_key is less
     *   than other's
     */
    bool operator<(const chunk_reader& other) const
    {
        return postings_ < other.postings_;
    }

    /**
     * Reads the next postings data from the stream.
     */
    void operator++()
    {
        bytes_read_ += postings_.read_packed(*file_);
    }

    /**
     * @return the total number of bytes read so far
     */
    uint64_t bytes_read() const
    {
        return bytes_read_;
    }

    /**
     * @return the total number of bytes in the chunk file
     */
    uint64_t total_bytes() const
    {
        return total_bytes_;
    }

    /**
     * @return the current buffered postings object
     */
    const PostingsData& postings() const
    {
        return postings_;
    }
};

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
    to_merge.reserve(std::distance(begin, end));
    for (; begin != end; ++begin)
        to_merge.emplace_back(*begin);

    printing::progress progress{
        " > Merging postings: ",
        std::accumulate(to_merge.begin(), to_merge.end(), 0ul,
                        [](uint64_t acc, const input_chunk& chunk)
                        {
                            return acc + chunk.total_bytes();
                        })};

    uint64_t unique_primary_keys = 0;

    uint64_t total_read
        = std::accumulate(to_merge.begin(), to_merge.end(), 0ul,
                          [](uint64_t acc, const input_chunk& chunk)
                          {
                              return acc + chunk.bytes_read();
                          });
    while (!to_merge.empty())
    {
        progress(total_read);
        ++unique_primary_keys;

        std::sort(to_merge.begin(), to_merge.end());

        // gather all postings that match the smallest primary key, reading
        // a new postings from the corresponding file
        auto range = std::equal_range(to_merge.begin(), to_merge.end(),
                                      *to_merge.begin());
        auto min_pk = range.first->postings().primary_key();

        using count_t = typename PostingsData::count_t;
        std::vector<count_t> to_write;
        to_write.reserve(std::distance(range.first, range.second));
        std::for_each(range.first, range.second, [&](input_chunk& chunk)
                      {
                          to_write.emplace_back(chunk.postings().counts());
                          auto before = chunk.bytes_read();
                          ++chunk;
                          total_read += (chunk.bytes_read() - before);
                      });

        // merge them all into one big counts vector
        count_t counts;
        std::for_each(to_write.begin(), to_write.end(), [&](count_t& pd)
                      {
                          std::move(pd.begin(), pd.end(),
                                    std::back_inserter(counts));
                          count_t{}.swap(pd);
                      });

        // write out the merged counts
        PostingsData output{std::move(min_pk)};
        output.set_counts(std::move(counts));
        output.write_packed(outstream);

        // remove all empty chunks from the input
        to_merge.erase(std::remove_if(to_merge.begin(), to_merge.end(),
                                      [](const input_chunk& chunk)
                                      {
                                          return !chunk;
                                      }),
                       to_merge.end());
    }
    return unique_primary_keys;
}
}
}
#endif
