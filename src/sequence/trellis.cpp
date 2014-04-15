/**
 * @file trellis.cpp
 * @author Chase Geigle
 */

#include "sequence/trellis.h"

namespace meta
{
namespace sequence
{

trellis::trellis(uint64_t size) : trellis_(size)
{
    // nothing
}

uint64_t trellis::size() const
{
    return trellis_.size();
}

void trellis::probability(uint64_t idx, const tag_t& tag, double prob)
{
    trellis_[idx][tag] = prob;
}

double trellis::probability(uint64_t idx, const tag_t& tag) const
{
    auto it = trellis_[idx].find(tag);
    if (it == trellis_[idx].end())
        return 0;
    return it->second;
}

auto trellis::begin(uint64_t idx) -> column_iterator
{
    return trellis_[idx].begin();
}

auto trellis::end(uint64_t idx) -> column_iterator
{
    return trellis_[idx].end();
}

viterbi_trellis::viterbi_trellis(uint64_t size) : trellis{size}, paths_(size)
{
    // nothing
}

void viterbi_trellis::previous_tag(uint64_t idx, const tag_t& current,
                                   const tag_t& previous)
{
    paths_[idx][current] = previous;
}

const tag_t& viterbi_trellis::previous_tag(uint64_t idx, const tag_t& current)
{
    return paths_[idx][current];
}

forward_trellis::forward_trellis(uint64_t size)
    : trellis{size}, normalizers_(size)
{
    // nothing
}

double forward_trellis::normalizer(uint64_t idx) const
{
    return normalizers_[idx];
}

void forward_trellis::normalize(uint64_t idx, double value)
{
    for (auto it = begin(idx); it != end(idx); ++it)
        it->second *= value;
    normalizers_[idx] = value;
}
}
}
