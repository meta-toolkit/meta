/**
 * @file debinarizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cassert>

#include "meta/parser/trees/visitors/debinarizer.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"

namespace meta
{
namespace parser
{

std::unique_ptr<node> debinarizer::operator()(const leaf_node& ln)
{
    return make_unique<leaf_node>(ln);
}

std::unique_ptr<node> debinarizer::operator()(const internal_node& in)
{

    auto res = make_unique<internal_node>(in.category());

    in.each_child(
        [&](const node* child)
        {
            auto n = child->accept(*this);
            if (n->is_temporary())
            {
                n->as<internal_node>().each_child(
                    [&](const node* c)
                    {
                        res->add_child(c->clone());
                        if (child == in.head_constituent()
                            && c == n->as<internal_node>().head_constituent())
                        {
                            res->head(res->child(res->num_children() - 1));
                        }
                    });
            }
            else
            {
                res->add_child(std::move(n));
                if (child == in.head_constituent())
                    res->head(res->child(res->num_children() - 1));
            }
        });

    return std::move(res);
}
}
}
