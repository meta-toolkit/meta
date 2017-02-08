/**
 * @file cooccur_iterator.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_EMBEDDINGS_COOCCUR_ITERATOR_H_
#define META_EMBEDDINGS_COOCCUR_ITERATOR_H_

#include "meta/config.h"
#include "meta/embeddings/cooccur_record.h"
#include "meta/util/multiway_merge.h"

namespace meta
{
namespace embeddings
{
/**
 * An iterator over cooccur_records that live in a packed file on disk.
 * Satisfies the ChunkIterator concept for multiway_merge support.
 */
using cooccur_iterator = util::chunk_iterator<cooccur_record>;
using destructive_cooccur_iterator
    = util::destructive_chunk_iterator<cooccur_record>;
}
}
#endif
