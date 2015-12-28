/**
 * @file observation.cpp
 * @author Chase Geigle
 */

#include "meta/sequence/observation.h"

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
    throw observation_exception{"no tag for this observation"};
}

const label_id& observation::label() const
{
    if (label_)
        return *label_;
    throw observation_exception{"no label for this observation"};
}

void observation::symbol(symbol_t sym)
{
    symbol_ = std::move(sym);
}

void observation::tag(tag_t t)
{
    tag_ = std::move(t);
}

void observation::label(label_id lbl)
{
    label_ = std::move(lbl);
}

bool observation::tagged() const
{
    return static_cast<bool>(tag_);
}

auto observation::features() const -> const feature_vector &
{
    return features_;
}

void observation::features(feature_vector feats)
{
    features_ = std::move(feats);
}
}
}
