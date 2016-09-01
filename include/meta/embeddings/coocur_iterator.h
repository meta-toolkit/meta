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

#include "meta/config.h"
#include "meta/embeddings/coocur_record.h"
#include "meta/util/multiway_merge.h"

namespace meta
{
namespace embeddings
{
/**
 * An iterator over coocur_record's that live in a packed file on disk.
 * Satisfies the ChunkIterator concept for multiway_merge support.
 */
using coocur_iterator = util::chunk_iterator<coocur_record>;
}
}
#endif
