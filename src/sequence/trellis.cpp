/**
 * @file trellis.cpp
 * @author Chase Geigle
 */

#include "sequence/trellis.h"

namespace meta
{
namespace sequence
{

trellis::trellis(uint64_t size, uint64_t labels) : trellis_(size)
{
    for (auto & v : trellis_)
    {
        v.resize(labels);
        std::fill(v.begin(), v.end(), 0);
    }
}

uint64_t trellis::size() const
{
    return trellis_.size();
}

void trellis::probability(uint64_t idx, const label_id& tag, double prob)
{
    trellis_[idx][tag] = prob;
}

double trellis::probability(uint64_t idx, const label_id& tag) const
{
    return trellis_[idx][tag];
}

viterbi_trellis::viterbi_trellis(uint64_t size, uint64_t labels)
    : trellis{size, labels}, paths_(size)
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

forward_trellis::forward_trellis(uint64_t size, uint64_t labels)
    : trellis{size, labels}, normalizers_(size)
{
    // nothing
}

double forward_trellis::normalizer(uint64_t idx) const
{
    return normalizers_[idx];
}

void forward_trellis::normalize(uint64_t idx, double value)
{
    for (auto & val : trellis_[idx])
        val *= value;
    normalizers_[idx] = value;
}
}
}
