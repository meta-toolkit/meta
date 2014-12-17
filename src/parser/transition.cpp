/**
 * @file transition.cpp
 * @author Chase Geigle
 */

#include "parser/transition.h"

namespace meta
{
namespace parser
{

transition::transition(type_t t) : type_{t}
{
    // nothing
}

transition::transition(type_t t, class_label lbl)
    : type_{t}, label_{std::move(lbl)}
{
    // nothing
}

auto transition::type() const -> type_t
{
    return type_;
}

const class_label& transition::label() const
{
    if (!label_)
        throw exception{"label not set on transition"};
    return *label_;
}

bool transition::operator<(const transition& rhs) const
{
    return std::tie(type_, label_) < std::tie(rhs.type_, rhs.label_);
}
}
}
