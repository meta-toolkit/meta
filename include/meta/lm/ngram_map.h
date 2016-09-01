/**
 * @file ngram_map.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LM_NGRAM_MAP_H_
#define META_LM_NGRAM_MAP_H_

#include <cstdint>

#include "meta/config.h"
#include "meta/hashing/perfect_hash_map.h"
#include "meta/io/packed.h"

namespace meta
{
namespace lm
{
template <class Prob = float, class Backoff = float>
struct prob_backoff
{
    Prob prob;
    Backoff backoff;
};

template <class OutputStream, class Prob, class Backoff>
uint64_t packed_write(OutputStream& os,
                      const lm::prob_backoff<Prob, Backoff>& pb)
{
    return io::packed::write(os, pb.prob) + io::packed::write(os, pb.backoff);
}

template <class InputStream, class Prob, class Backoff>
uint64_t packed_read(InputStream& is, lm::prob_backoff<Prob, Backoff>& pb)
{
    return io::packed::read(is, pb.prob) + io::packed::read(is, pb.backoff);
}

template <class KeyType, class ValueType = prob_backoff<>,
          class FingerPrint = uint32_t>
using ngram_map_builder
    = hashing::perfect_hash_map_builder<KeyType, ValueType, FingerPrint>;

template <class KeyType, class ValueType = prob_backoff<>,
          class FingerPrint = uint32_t>
using ngram_map = hashing::perfect_hash_map<KeyType, ValueType, FingerPrint>;
}
}
#endif
