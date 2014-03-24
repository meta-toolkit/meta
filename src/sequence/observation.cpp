/**
 * @file sequence.cpp
 * @author Chase Geigle
 */

#include "sequence/observation.h"

namespace meta
{
namespace sequence
{

observation::observation(symbol_t sym, tag_t t)
    : symbol_{std::move(sym)}, tag_{std::move(t)}
{
    // nothing
}

observation::observation(symbol_t sym)
    : symbol_{std::move(sym)}, tag_{util::nullopt}
{
    // nothing
}

const symbol_t& observation::symbol() const
{
    return symbol_;
}

const tag_t& observation::tag() const
{
    if (tag_)
        return *tag_;
    throw exception{"no tag for this observation"};
}

void observation::symbol(symbol_t sym)
{
    symbol_ = std::move(sym);
}

void observation::tag(tag_t t)
{
    tag_ = std::move(t);
}

bool observation::tagged() const
{
    return static_cast<bool>(tag_);
}
}
}
