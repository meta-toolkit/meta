/**
 * @file trellis.cpp
 * @author Chase Geigle
 */

#include <numeric>
#include <algorithm>
#include "sequence/trellis.h"

namespace meta
{
namespace sequence
{

trellis::trellis(uint64_t size, uint64_t labels)
    : trellis_(size * labels, 0), labels_{labels}
{
    // nothing
}

uint64_t trellis::size() const
{
    return trellis_.size() / labels_;
}

void trellis::probability(uint64_t idx, const label_id& tag, double prob)
{
    trellis_[idx * labels_ + tag] = prob;
}

double trellis::probability(uint64_t idx, const label_id& tag) const
{
    return trellis_[idx * labels_ + tag];
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

void forward_trellis::normalize(uint64_t idx)
{
    auto beg = trellis_.begin() + idx * labels_;
    auto end = trellis_.begin() + (idx + 1) * labels_;
    auto sum = std::accumulate(beg, end, 0.0);
    auto normalizer = sum != 0 ? 1.0 / sum : 1;

    std::transform(beg, end, beg, [&](double val) { return val * normalizer; });

    normalizers_[idx] = normalizer;
}
}
}