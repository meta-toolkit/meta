/**
 * @file cooccur_record.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMBEDDINGS_COOCCUR_RECORD_H_
#define META_EMBEDDINGS_COOCCUR_RECORD_H_

#include <cstdint>

#include "meta/config.h"
#include "meta/io/packed.h"

namespace meta
{
namespace embeddings
{
/**
 * Represents an entry in the cooccurrence matrix. Satisfies the Record
 * concept for multiway_merge support.
 */
struct cooccur_record
{
    uint64_t target;
    uint64_t context;
    double weight;

    void merge_with(cooccur_record&& other)
    {
        weight += other.weight;
    }
};

inline bool operator==(const cooccur_record& a, const cooccur_record& b)
{
    return std::tie(a.target, a.context) == std::tie(b.target, b.context);
}

inline bool operator!=(const cooccur_record& a, const cooccur_record& b)
{
    return !(a == b);
}

inline bool operator<(const cooccur_record& a, const cooccur_record& b)
{
    return std::tie(a.target, a.context) < std::tie(b.target, b.context);
}

template <class OutputStream>
uint64_t packed_write(OutputStream& os, const cooccur_record& record)
{
    using io::packed::write;
    return write(os, record.target) + write(os, record.context)
           + write(os, record.weight);
}

template <class InputStream>
uint64_t packed_read(InputStream& is, cooccur_record& record)
{
    using io::packed::read;
    return read(is, record.target) + read(is, record.context)
           + read(is, record.weight);
}
}
}
#endif
