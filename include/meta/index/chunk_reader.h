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

#include "meta/io/filesystem.h"
#include "meta/util/progress.h"
#include "meta/util/shim.h"
#include "meta/util/multiway_merge.h"

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
class chunk_reader
{
  private:
    /// the file we're reading from currently, or null if there is none
    std::unique_ptr<std::ifstream> file_;
    /// the path to the file we're reading from
    std::string path_;
    /// the current buffered postings data
    postings_record<PostingsData> postings_;
    /// the total number of bytes in the chunk we're reading
    uint64_t total_bytes_;
    /// the total number of bytes read
    uint64_t bytes_read_;

  public:
    using value_type = postings_record<PostingsData>;

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
     * Constructs an empty chunk reader.
     */
    chunk_reader() : total_bytes_{0}, bytes_read_{0}
    {
        // nothing
    }

    chunk_reader(chunk_reader&&) = default;

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
     * Reads the next postings data from the stream.
     */
    void operator++()
    {
        bytes_read_ += postings_.read(*file_);
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
    postings_record<PostingsData>& operator*()
    {
        return postings_;
    }

    /**
     * @return the current buffered postings object
     */
    const postings_record<PostingsData>& operator*() const
    {
        return postings_;
    }

    /**
     * Whether this chunk_reader is equal to another. chunk_readers are
     * equal if they are both exhausted or both are reading from the same
     * path and have read the same number of bytes.
     */
    bool operator==(const chunk_reader& other) const
    {
        if (!other.file_)
        {
            return !file_ || !static_cast<bool>(*file_);
        }
        else
        {
            return std::tie(path_, bytes_read_)
                   == std::tie(other.path_, bytes_read_);
        }
    }
};

/**
 * Whether two chunk_readers differ. Defined in terms of operator==.
 */
template <class PostingsData>
bool operator!=(const chunk_reader<PostingsData>& a,
                const chunk_reader<PostingsData>& b)
{
    return !(a == b);
}

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

    return util::multiway_merge(to_merge.begin(), to_merge.end(),
                                [&](PostingsData&& pdata)
                                {
                                    pdata.write_packed(outstream);
                                });
}
}
}
#endif
