/**
 * @file node.cpp
 * @author Chase Geigle
 */

#include "parser/tree/node.h"

namespace meta
{
namespace parser
{

node::node(class_label cat) : category_{std::move(cat)}
{
    // nothing
}

const class_label& node::category() const
{
    return category_;
}

bool node::is_leaf() const
{
    return false;
}
}
}
