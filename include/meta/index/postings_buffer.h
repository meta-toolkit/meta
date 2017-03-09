/**
 * @file postings_buffer.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_POSTINGS_BUFFER_H_
#define META_INDEX_POSTINGS_BUFFER_H_

#include <array>
#include <cstdint>
#include <forward_list>
#include <memory>
#include <vector>

#include "meta/config.h"
#include "meta/index/postings_stream.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/util/arena_allocator.h"
#include "meta/util/shim.h"

namespace meta
{
namespace index
{
namespace detail
{
template <class T>
struct bytes_used
{
    std::size_t operator()(const T&) const
    {
        return 0;
    }
};

template <class Char, class Traits, class Allocator>
struct bytes_used<std::basic_string<Char, Traits, Allocator>>
{
    using string_type = std::basic_string<Char, Traits, Allocator>;

    std::size_t operator()(const string_type& str) const
    {
        // this function attempts to figure out how many heap-allocated
        // bytes are in use by a particular string in the presence of the
        // small string optimization.
        const static string_type empty_string;
        if (str.capacity() > empty_string.capacity())
            return sizeof(Char) * str.capacity();
        return 0;
    }
};
}

/**
 * Represents the postings list for an in-memory chunk assocated with a
 * specific PrimaryKey (usually a std::string). Each postings_buffer stores
 * the PrimaryKey, the total number of (key, value) pairs in the postings
 * list, the total sum of the counts in the postings list, and a byte
 * buffer that holds the compressed form of the postings list itself. This
 * allows us to store significantly larger in-memory chunks than if we were
 * to store the full materialized postings_data.
 *
 * The byte buffer is implemented as a std::forward_list of (compile-time)
 * fixed-size chunks on top of a thread-local (run-time) fixed-size arena.
 * This works for us in this case because (1) in-memory postings tend to be
 * very small and (2) using an arena allocator helps reduce memory
 * fragmentation problems that arise if using a std::vector-style byte
 * buffer.
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue = uint64_t>
class postings_buffer
{
  public:
    using byte_type = uint8_t;

    // we use 31 bytes per buffer to ensure that we have one byte left over
    // to store the current write index into that buffer; each allocation
    // will then take 32 + sizeof(void*) bytes after factoring in the next
    // pointer of the list node
    using buffer_type = std::array<byte_type, 31>;

    /**
     * A byte buffer on top of a fixed-size arena.
     */
    class char_buffer
    {
      public:
        /**
         * This is the storage for a single node in the linked list
         * associated with this postings buffer.
         */
        class node
        {
          public:
            bool full() const
            {
                return pos_ == buffer_.size();
            }

            uint8_t size() const
            {
                return pos_;
            }

            void reset(uint8_t sz)
            {
                pos_ = sz;
            }

            void put(char byte)
            {
                assert(pos_ < buffer_.size());
                buffer_[pos_] = static_cast<uint8_t>(byte);
                ++pos_;
            }

            byte_type operator[](uint8_t idx) const
            {
                return buffer_[idx];
            }

            template <class OutputStream>
            uint64_t write(OutputStream& os) const
            {
                os.write(reinterpret_cast<const char*>(buffer_.data()),
                         static_cast<std::streamsize>(pos_));
                return pos_;
            }

          private:
            buffer_type buffer_;
            uint8_t pos_ = 0;
        };

        using allocator_type = util::arena_allocator<node, util::arena<8>>;
        using list_type = std::forward_list<node, allocator_type>;

        char_buffer(const allocator_type& alloc)
            : flist_(alloc), tail_{flist_.before_begin()}
        {
            // nothing
        }

        void write_count(SecondaryKey gap, FeatureValue count)
        {
            if (flist_.empty())
            {
                tail_ = flist_.insert_after(tail_, node{});
                ++size_;
            }

            const auto pos = tail_->size();
            const auto old_tail = tail_;
            try
            {
                io::packed::write(*this, gap);
                io::packed::write(*this, count);
            }
            catch (const util::arena_bad_alloc&)
            {
                // if we failed to perform the full write, we need to
                // (1) reset the position of the node that used to be tail
                // (2) erase any new nodes that might have been allocated
                // (3) reset the tail to the old node
                // (4) re-throw the exception
                old_tail->reset(pos);
                flist_.erase_after(old_tail, flist_.end());
                tail_ = old_tail;
                throw;
            }
        }

        void put(char byte)
        {
            if (tail_->full())
            {
                tail_ = flist_.insert_after(tail_, node{});
                ++size_;
            }

            tail_->put(byte);
        }

        template <class OutputStream>
        uint64_t write(OutputStream& os) const
        {
            uint64_t total_bytes = 0;
            for (const auto& n : flist_)
                total_bytes += n.write(os);
            return total_bytes;
        }

        uint64_t bytes_used() const
        {
            return (sizeof(node) + sizeof(void*)) * size_;
        }

        /**
         * Emulates an input stream on top of the char buffer's linked
         * list that is suitable for using as input to io::packed::read
         * operations.
         */
        class stream
        {
          public:
            stream(typename list_type::const_iterator begin,
                   typename list_type::const_iterator end)
                : it_{begin}, end_{end}
            {
                // nothing
            }

            int peek() const
            {
                if (it_ == end_)
                    return EOF;
                return static_cast<unsigned char>((*it_)[pos_]);
            }

            int get()
            {
                if (it_ == end_)
                    return EOF;
                auto ret = static_cast<unsigned char>((*it_)[pos_]);
                ++pos_;
                if (pos_ == it_->size())
                {
                    ++it_;
                    pos_ = 0;
                }
                return ret;
            }

            bool operator==(const stream& other) const
            {
                return std::tie(it_, end_, pos_)
                       == std::tie(other.it_, other.end_, other.pos_);
            }

            bool operator!=(const stream& other) const
            {
                return !(*this == other);
            }

          private:
            typename list_type::const_iterator it_;
            const typename list_type::const_iterator end_;
            uint8_t pos_ = 0;
        };

        /**
         * Iterates the (key, value) pairs in a char_buffer.
         */
        class iterator
        {
          public:
            using value_type = std::pair<SecondaryKey, FeatureValue>;
            using reference = const value_type&;
            using pointer = const value_type*;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::input_iterator_tag;

            iterator(typename list_type::const_iterator begin,
                     typename list_type::const_iterator end)
                : stream_{begin, end},
                  value_{std::make_pair(SecondaryKey{0}, FeatureValue{0})}
            {
                ++(*this);
            }

            iterator& operator++()
            {
                if (stream_.peek() == EOF)
                {
                    at_end_ = true;
                    return *this;
                }

                uint64_t gap;
                io::packed::read(stream_, gap);
                value_.first += gap;
                io::packed::read(stream_, value_.second);
                return *this;
            }

            std::pair<SecondaryKey, FeatureValue>& operator*()
            {
                return value_;
            }

            const std::pair<SecondaryKey, FeatureValue>& operator*() const
            {
                return value_;
            }

            bool operator==(const iterator& other) const
            {
                return std::tie(stream_, at_end_)
                       == std::tie(other.stream_, other.at_end_);
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

          private:
            stream stream_;
            std::pair<SecondaryKey, FeatureValue> value_;
            bool at_end_ = false;
        };

        iterator begin()
        {
            return {flist_.begin(), flist_.end()};
        }

        iterator begin() const
        {
            return {flist_.begin(), flist_.end()};
        }

        iterator end()
        {
            return {flist_.end(), flist_.end()};
        }

        iterator end() const
        {
            return {flist_.end(), flist_.end()};
        }

      private:
        list_type flist_;
        typename list_type::iterator tail_;
        std::size_t size_ = 0;
    };

    using allocator_type = typename char_buffer::allocator_type;
    using iterator = typename char_buffer::iterator;

    iterator begin()
    {
        return buffer_.begin();
    }

    iterator begin() const
    {
        return buffer_.begin();
    }

    iterator end()
    {
        return buffer_.end();
    }

    iterator end() const
    {
        return buffer_.end();
    }

    std::size_t size() const
    {
        return num_ids_;
    }

    /**
     * Creates a postings_buffer for a specific primary key.
     */
    postings_buffer(PrimaryKey pk, const allocator_type& alloc)
        : buffer_(alloc), pk_(std::move(pk)), alloc_(alloc)
    {
        // nothing
    }

    /**
     * Moves a postings_buffer.
     */
    postings_buffer(postings_buffer&& other)
        : buffer_{std::move(other.buffer_)},
          pk_{std::move(other.pk_)},
          last_id_{other.last_id_},
          num_ids_{other.num_ids_},
          total_counts_{other.total_counts_},
          alloc_{other.alloc_}
    {
        // nothing
    }

    /**
     * Move-assigns a postings_buffer.
     */
    postings_buffer& operator=(postings_buffer&& other)
    {
        assert(other.alloc_ == alloc_);
        buffer_ = std::move(other.buffer_);
        pk_ = std::move(other.pk_);
        last_id_ = other.last_id_;
        num_ids_ = other.num_ids_;
        total_counts_ = other.total_counts_;
        return *this;
    }

    /**
     * @return the primary key for this postings_buffer
     */
    const PrimaryKey& primary_key() const
    {
        return pk_;
    }

    /**
     * Writes a postings entry to the in-memory byte buffer in compressed
     * format.
     * @param id The SecondaryKey for the pair
     * @param count The count value associated with the id
     */
    void write_count(SecondaryKey id, FeatureValue count)
    {
        assert(id >= last_id_);
        buffer_.write_count(id - last_id_, count);

        ++num_ids_;
        total_counts_ += count;
        last_id_ = id;
    }

    /**
     * @return an estimate of the number of heap allocated bytes this
     * structure uses
     */
    std::size_t bytes_used() const
    {
        auto bytes = buffer_.bytes_used();
        bytes += detail::bytes_used<PrimaryKey>{}(pk_);
        return bytes;
    }

    /**
     * Writes this buffer directly to an output stream.
     * @param os The output stream to write to
     * @return the number of bytes written
     */
    template <class OutputStream>
    uint64_t write_packed(OutputStream& os)
    {
        auto bytes = io::packed::write(os, pk_);
        bytes += io::packed::write(os, num_ids_);
        bytes += io::packed::write(os, total_counts_);

        return bytes + buffer_.write(os);
    }

    /**
     * @return a postings_stream to iterate over the byte buffer
     */
    postings_stream<SecondaryKey, FeatureValue> stream() const
    {
        return {reinterpret_cast<const char*>(buffer_.bytes_.get()), num_ids_,
                total_counts_};
    }

    /**
     * @param rhs The other buffer
     * @return whether the primary key of this buffer is less than the
     * primary key of the other buffer
     */
    bool operator<(const postings_buffer& rhs) const
    {
        return pk_ < rhs.pk_;
    }

    /**
     * @param rhs The other buffer
     * @return whether the primary keys of the two buffers are equal
     */
    bool operator==(const postings_buffer& rhs) const
    {
        return pk_ == rhs.pk_;
    }

  private:
    /// The storage for the in-memory postings
    char_buffer buffer_;
    /// The primary key for the buffer
    PrimaryKey pk_;
    /// The last id we wrote
    SecondaryKey last_id_ = SecondaryKey{0};
    /// The total number of ids we've written
    uint64_t num_ids_ = 0;
    /// The sum of the counts we've written
    FeatureValue total_counts_ = 0;
    /// The allocator to use for list nodes in the buffer
    allocator_type alloc_;
};

template <class HashAlgorithm, class PrimaryKey, class SecondaryKey>
void hash_append(HashAlgorithm& h,
                 const postings_buffer<PrimaryKey, SecondaryKey>& pb)
{
    using hashing::hash_append;
    hash_append(h, pb.primary_key());
}
}
}

namespace std
{
template <class PrimaryKey, class SecondaryKey>
struct hash<meta::index::postings_buffer<PrimaryKey, SecondaryKey>>
{
    using pbuffer_type = meta::index::postings_buffer<PrimaryKey, SecondaryKey>;
    std::size_t operator()(const pbuffer_type& pbuffer) const
    {
        return std::hash<PrimaryKey>{}(pbuffer.primary_key());
    }
};
}
#endif
