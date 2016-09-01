/**
 * @file postings_stream.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_POSTINGS_STREAM_H_
#define META_INDEX_POSTINGS_STREAM_H_

#include <iterator>
#include <tuple>
#include <utility>

#include "meta/config.h"
#include "meta/io/packed.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

/**
 * A stream for extracting the postings list for a specific key in a
 * postings file. This can be used instead of postings_data to avoid
 * reading in the entire postings list into memory at once.
 */
template <class SecondaryKey, class FeatureValue = uint64_t>
class postings_stream
{
  private:
    struct char_input_stream
    {
        char_input_stream(const char* input) : input_{input}
        {
            // nothing
        }

        char get()
        {
            return *input_++;
        }

        const char* input_;
    };

  public:
    /**
     * Creates a postings stream reading from the given buffer. Assumes
     * that the size and total counts are the first two values in the
     * buffer.
     *
     * @param buffer The buffer position to the start of the postings
     */
    postings_stream(const char* buffer) : start_{buffer}
    {
        char_input_stream stream{start_};

        io::packed::read(stream, size_);
        io::packed::read(stream, total_counts_);
        start_ = stream.input_;
    }

    /**
     * Creates a postings stream reading from the given buffer. Assumes
     * that the very first value in the buffer is the start of the
     * postings, since the size and total counts are provided on
     * construction.
     */
    postings_stream(const char* buffer, uint64_t size,
                    FeatureValue total_counts)
        : start_{buffer}, size_{size}, total_counts_{total_counts}
    {
        // nothing
    }

    /**
     * @return the number of SecondaryKeys in this postings list.
     */
    uint64_t size() const
    {
        return size_;
    }

    /**
     * @return the total sum of the counts for SecondaryKeys in this
     * postings list.
     */
    FeatureValue total_counts() const
    {
        return total_counts_;
    }

    /**
     * Writes this postings stream to an output stream in packed format.
     * @return the number of bytes written
     */
    template <class OutputStream>
    uint64_t write_packed(OutputStream& os) const
    {
        auto bytes = io::packed::write(os, size_);
        bytes += io::packed::write(os, total_counts_);
        for (const auto& pr : *this)
        {
            bytes += io::packed::write(os, pr.first);
            bytes
                += io::packed::write(os, static_cast<FeatureValue>(pr.second));
        }
        return bytes;
    }

    /**
     * An iterator over the (SecondaryKey, FeatureValue) pairs of this postings
     * list.
     */
    class iterator
    {
      public:
        using value_type = std::pair<SecondaryKey, FeatureValue>;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;

        friend postings_stream;

        iterator() : stream_{nullptr}, size_{0}, pos_{0}
        {
            // nothing
        }

        iterator& operator++()
        {
            if (pos_ == size_)
            {
                stream_ = {nullptr};
                size_ = 0;
                pos_ = 0;
            }
            else
            {
                uint64_t id;
                io::packed::read(stream_, id);
                // gap encoding
                count_.first += id;
                io::packed::read(stream_, count_.second);
                ++pos_;
            }
            return *this;
        }

        util::optional<value_type> operator++(int)
        {
            auto proxy = *(*this);
            ++(*this);
            return proxy;
        }

        reference operator*() const
        {
            return count_;
        }

        pointer operator->() const
        {
            return &count_;
        }

        bool operator==(const iterator& other)
        {
            return std::tie(stream_.input_, size_, pos_)
                   == std::tie(other.stream_.input_, other.size_, other.pos_);
        }

        bool operator!=(const iterator& other)
        {
            return !(*this == other);
        }

      private:
        iterator(const char* start, uint64_t size)
            : stream_{start},
              size_{size},
              pos_{0},
              count_{std::make_pair(SecondaryKey{0}, 0.0)}
        {
            ++(*this);
        }

        char_input_stream stream_;
        uint64_t size_;
        uint64_t pos_;
        value_type count_;
    };

    /**
     * @return an iterator to the beginning of the list
     */
    iterator begin() const
    {
        return {start_, size_};
    }

    /**
     * @return an iterator to the ending of the list
     */
    iterator end() const
    {
        return {};
    }

  private:
    const char* start_;
    uint64_t size_;
    FeatureValue total_counts_;
};
}
}
#endif
