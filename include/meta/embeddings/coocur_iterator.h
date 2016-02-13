/**
 * @file coocur_iterator.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMBEDDINGS_COOCUR_ITERATOR_H_
#define META_EMBEDDINGS_COOCUR_ITERATOR_H_

#include <fstream>

#include "meta/embeddings/coocur_record.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"

namespace meta
{
namespace embeddings
{

/**
 * An iterator over coocur_record's that live in a packed file on disk.
 * Satisfies the ChunkIterator concept for multiway_merge support.
 */
class coocur_iterator
{
  public:
    using value_type = coocur_record;

    coocur_iterator(const std::string& filename)
        : path_{filename},
          input_{make_unique<std::ifstream>(filename, std::ios::binary)},
          total_bytes_{filesystem::file_size(filename)},
          bytes_read_{0}
    {
        ++(*this);
    }

    coocur_iterator() = default;
    coocur_iterator(coocur_iterator&&) = default;

    coocur_iterator& operator++()
    {
        if (input_->peek() == EOF)
            return *this;

        bytes_read_ += record_.read(*input_);
        return *this;
    }

    coocur_record& operator*()
    {
        return record_;
    }

    const coocur_record& operator*() const
    {
        return record_;
    }

    bool operator==(const coocur_iterator& other) const
    {
        if (!other.input_)
        {
            return !input_ || !static_cast<bool>(*input_);
        }
        else
        {
            return std::tie(path_, bytes_read_)
                   == std::tie(other.path_, other.bytes_read_);
        }
    }

    uint64_t total_bytes() const
    {
        return total_bytes_;
    }

    uint64_t bytes_read() const
    {
        return bytes_read_;
    }

  private:
    std::string path_;
    std::unique_ptr<std::ifstream> input_;
    coocur_record record_;
    uint64_t total_bytes_;
    uint64_t bytes_read_;
};

bool operator!=(const coocur_iterator& a, const coocur_iterator& b)
{
    return !(a == b);
}
}
}
#endif
