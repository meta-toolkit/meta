/**
 * @file node.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/node.h"

namespace meta
{
namespace parser
{

namespace
{
bool is_temporary(const std::string& lbl)
{
    return lbl[lbl.length() - 1] == '*';
}
}

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

bool node::is_temporary() const
{
    if (is_leaf())
        return false;

    return ::meta::parser::is_temporary(category());
}
}
}
