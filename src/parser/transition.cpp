/**
 * @file transition.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <ostream>
#include <string>
#include "meta/parser/transition.h"

namespace meta
{
namespace parser
{

transition::transition(type_t t) : type_{t}
{
    switch (type_)
    {
        case transition::type_t::SHIFT:
        case transition::type_t::FINALIZE:
        case transition::type_t::IDLE:
            return;
        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
        case transition::type_t::UNARY:
            throw exception{
                "label required for REDUCE_L, REDUCE_R, or UNARY transitions"};
        default:
            throw exception{"unrecognized transition type"};
    }
}

transition::transition(type_t t, class_label lbl)
    : type_{t}, label_{std::move(lbl)}
{
    switch (type_)
    {
        case transition::type_t::SHIFT:
        case transition::type_t::FINALIZE:
        case transition::type_t::IDLE:
            throw exception{"SHIFT, FINALIZE, or IDLE actions are not allowed "
                            "to have labels"};
        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
        case transition::type_t::UNARY:
            return;
        default:
            throw exception{"unrecognized transition type"};
    }
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

std::ostream& operator<<(std::ostream& os, transition::type_t type)
{
    switch (type)
    {
        case transition::type_t::SHIFT:
            os << "SHIFT";
            break;

        case transition::type_t::REDUCE_L:
            os << "REDUCE-L";
            break;

        case transition::type_t::REDUCE_R:
            os << "REDUCE-R";
            break;

        case transition::type_t::UNARY:
            os << "UNARY";
            break;

        case transition::type_t::FINALIZE:
            os << "FINALIZE";
            break;

        case transition::type_t::IDLE:
            os << "IDLE";
            break;

        default:
            os << "ERROR-" << (int)type;
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const transition& trans)
{
    os << trans.type();
    switch (trans.type())
    {
        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
        case transition::type_t::UNARY:
            os << "-" << trans.label();
            break;

        default:
            break;
    }
    return os;
}
}
}
