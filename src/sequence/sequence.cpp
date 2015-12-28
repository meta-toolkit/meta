/**
 * @file sequence.cpp
 * @author Chase Geigle
 */

#include "meta/sequence/sequence.h"

namespace meta
{
namespace sequence
{

void sequence::add_observation(observation obs)
{
    observations_.emplace_back(std::move(obs));
}

void sequence::add_symbol(symbol_t sym)
{
    observations_.emplace_back(std::move(sym));
}

const observation& sequence::operator[](size_type idx) const
{
    return observations_[idx];
}

observation& sequence::operator[](size_type idx)
{
    return observations_[idx];
}

auto sequence::begin() -> iterator
{
    return observations_.begin();
}

auto sequence::end() -> iterator
{
    return observations_.end();
}

auto sequence::begin() const -> const_iterator
{
    return observations_.begin();
}

auto sequence::end() const -> const_iterator
{
    return observations_.end();
}

auto sequence::size() const -> size_type
{
    return observations_.size();
}
}
}
