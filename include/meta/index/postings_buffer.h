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

#include <cstdint>
#include <memory>
#include <vector>

#include "meta/config.h"
#include "meta/index/postings_stream.h"
#include "meta/io/packed.h"
#include "meta/util/shim.h"

namespace meta
{
namespace index
{

namespace detail
{
/**
 * Gets the bytes used by a std::string.
 */
template <class T>
uint64_t bytes_used(
    const T& elem,
    typename std::enable_if<std::is_same<T,
                                         std::string>::value>::type* = nullptr)
{
    return elem.capacity();
}

/**
 * Gets the bytes used by anything not a std::string.
 */
template <class T>
uint64_t bytes_used(
    const T& elem,
    typename std::enable_if<!std::is_same<T,
                                          std::string>::value>::type* = nullptr)
{
    return sizeof(elem);
}
}

/**
 * Represents the postings list for an in-memory chunk assocated with a
 * specific PrimaryKey (usually a std::string). Each postings_buffer stores
 * the PrimaryKey, the total number of key, value pairs in the postings
 * list, the total sum of the counts in the postings list,  and a byte
 * buffer that holds the compressed form of the postings list itself. This
 * allows us to store significantly larger in-memory chunks than if we were
 * to store the full materialized postings_data.
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue = uint64_t>
class postings_buffer
{
  private:
    using byte_type = uint8_t;
    using buffer_type = std::vector<byte_type>;
    using const_buffer_iterator = buffer_type::const_iterator;

    /// A simple input stream that reads from a buffer using an iterator
    struct buffer_input_stream
    {
        buffer_input_stream(const_buffer_iterator it) : it_{it}
        {
            // nothing
        }

        char get()
        {
            return *it_++;
        }

        const_buffer_iterator it_;
    };

  public:
    /**
     * Creates a postings_buffer for a specific primary key.
     */
    postings_buffer(PrimaryKey pk) : pk_(std::move(pk))
    {
        // nothing
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
        ++num_ids_;
        total_counts_ += count;

        assert(id >= last_id_);
        io::packed::write(buffer_, id - last_id_);
        io::packed::write(buffer_, count);

        last_id_ = id;
    }

    /**
     * @return an estimate of the number of heap allocated bytes this
     * structure uses
     */
    std::size_t bytes_used() const
    {
        auto bytes = buffer_.size_;

        // this only matters when PrimaryKey is std::string.
        // if the capacity of the string is bigger than the size of the
        // string itself, then we know it must also be using heap memory,
        // which we haven't accounted for already.
        if (detail::bytes_used(pk_) > sizeof(PrimaryKey))
            bytes += detail::bytes_used(pk_);
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

        buffer_.write(os);
        return bytes + buffer_.size_;
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
    /// A simple byte buffer that resizes with a 1.5x policy when full
    struct char_buffer
    {
        /// Constructs an empty buffer
        char_buffer() : size_{0}, pos_{0}
        {
        }

        /**
         * Copies an existing buffer
         * @param other The buffer to copy
         */
        char_buffer(const char_buffer& other)
            : size_{other.size_}, pos_{other.pos_}
        {
            if (other.bytes_)
            {
                bytes_ = make_unique<uint8_t[]>(size_);
                std::copy(other.bytes_.get(), other.bytes_.get() + pos_,
                          bytes_.get());
            }
        }

        /// char_buffer can be move constructed
        char_buffer(char_buffer&&) = default;

        /**
         * @param rhs The buffer to assign into this one
         * @return the current buffer
         */
        char_buffer& operator=(const char_buffer& rhs)
        {
            char_buffer copy{rhs};
            swap(copy);
            return *this;
        }

        /// char_buffer can be move assigned
        char_buffer& operator=(char_buffer&&) = default;

        /**
         * Swaps the current buffer with the argument
         * @param other The buffer to swap with
         */
        void swap(char_buffer& other)
        {
            using std::swap;
            swap(size_, other.size_);
            swap(pos_, other.pos_);
            swap(bytes_, other.bytes_);
        }

        /**
         * Writes a single byte to the buffer, resizing if needed.
         * @param byte the byte to write
         */
        void put(char byte)
        {
            if (size_ == pos_)
                resize();
            bytes_[pos_] = static_cast<uint8_t>(byte);
            ++pos_;
        }

        /**
         * Resizes the buffer to 1.5x its old size.
         */
        void resize()
        {
            if (size_ == 0)
            {
                size_ = 8;
            }
            else
            {
                // 1.5x resize
                size_ += (size_ + 1) / 2;
            }

            auto newbytes = make_unique<uint8_t[]>(size_);
            std::copy(bytes_.get(), bytes_.get() + pos_, newbytes.get());
            std::swap(newbytes, bytes_);
        }

        /**
         * Writes all the bytes in this buffer to the output stream
         * @param os The output stream to write to.
         */
        template <class OutputStream>
        void write(OutputStream& os) const
        {
            os.write(reinterpret_cast<const char*>(bytes_.get()),
                     static_cast<std::streamsize>(pos_));
        }

        /// The bytes in this buffer
        std::unique_ptr<uint8_t[]> bytes_;
        /// The current size of the buffer
        std::size_t size_;
        /// The current byte position in the buffer
        std::size_t pos_;

    } buffer_;

    /// The primary key for the buffer
    PrimaryKey pk_;
    /// The last id we wrote
    SecondaryKey last_id_ = SecondaryKey{0};
    /// The total number of ids we've written
    uint64_t num_ids_ = 0;
    /// The sum of the counts we've written
    FeatureValue total_counts_ = 0;
};

template <class HashAlgorithm, class PrimaryKey, class SecondaryKey>
void hash_append(HashAlgorithm& h,
                 const postings_buffer<PrimaryKey, SecondaryKey>& pb)
{
    using util::hash_append;
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
