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
#include <utility>

#include "io/mmap_file.h"
#include "io/compressed_file_reader.h"
#include "util/optional.h"
#include "io/packed.h"

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
     * Creates a postings stream reading from the given file at the given
     * byte position.
     *
     * @param file The file that contains the postings lists
     * @param seek_pos The position in the file to begin reading from
     */
    postings_stream(const io::mmap_file& file, uint64_t seek_pos)
        : file_{&file}, seek_pos_{seek_pos}
    {
        char_input_stream stream{file_->begin() + seek_pos_};

        io::packed::read(stream, size_);
        io::packed::read(stream, total_counts_);
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
    uint64_t total_counts() const
    {
        return total_counts_;
    }

    /**
     * An iterator over the (SecondaryKey, double) pairs of this postings
     * list.
     */
    class iterator
    {
      public:
        using value_type = std::pair<SecondaryKey, double>;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;

        friend postings_stream;

        iterator() : size_{0}, pos_{0}
        {
            // nothing
        }

        iterator& operator++()
        {
            if (stor_)
            {
                if (pos_ == size_)
                {
                    stor_ = util::nullopt;
                    pos_ = 0;
                    size_ = 0;
                }
                else
                {
                    uint64_t id;
                    io::packed::read(*stream_, id);
                    // gap encoding
                    stor_->first += id;

                    if (std::is_same<FeatureValue, uint64_t>::value)
                    {
                        uint64_t next;
                        io::packed::read(*stream_, next);
                        stor_->second = static_cast<double>(next);
                    }
                    else
                    {
                        io::packed::read(*stream_, stor_->second);
                    }
                    ++pos_;
                }
            }
            return *this;
        }

        util::optional<std::pair<SecondaryKey, double>> operator++(int)
        {
            auto proxy = *(*this);
            ++(*this);
            return proxy;
        }

        reference operator*() const
        {
            return *stor_;
        }

        pointer operator->() const
        {
            return &(*stor_);
        }

        bool operator==(const iterator& other)
        {
            return std::tie(stor_, size_, pos_)
                   == std::tie(other.stor_, other.size_, other.pos_);
        }

        bool operator!=(const iterator& other)
        {
            return !(*this == other);
        }

      private:
        iterator(const io::mmap_file& file, uint64_t seek_pos)
            : stream_{file.begin() + seek_pos},
              pos_{0},
              stor_{std::make_pair(SecondaryKey{0}, 0.0)}
        {
            io::packed::read(*stream_, size_);

            // ignore total counts
            uint64_t total_counts;
            io::packed::read(*stream_, total_counts);
            ++(*this);
        }

        util::optional<char_input_stream> stream_;
        uint64_t size_;
        uint64_t pos_;
        util::optional<std::pair<SecondaryKey, double>> stor_;
    };

    /**
     * @return an iterator to the beginning of the list
     */
    iterator begin() const
    {
        return {*file_, seek_pos_};
    }

    /**
     * @return an iterator to the ending of the list
     */
    iterator end() const
    {
        return {};
    }

  private:
    const io::mmap_file* file_;
    uint64_t seek_pos_;
    uint64_t size_;
    uint64_t total_counts_;
};
}
}
#endif
