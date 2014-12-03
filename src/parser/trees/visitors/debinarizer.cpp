/**
 * @file debinarizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cassert>

#include "parser/trees/visitors/debinarizer.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "util/shim.h"

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

    class_label bin_cat{static_cast<std::string>(in.category()) + "*"};
    in.each_child([&](const node* child)
                  {
                      debinarize_subtree(in, bin_cat, child, *res);
                  });
    return std::move(res);
}

void debinarizer::debinarize_subtree(const internal_node& in,
                                     const class_label& bin_cat,
                                     const node* subroot, internal_node& res)
{

    if (subroot->category() != bin_cat)
    {
        res.add_child(subroot->accept(*this));

        if (subroot == in.head_constituent())
            res.head(res.child(res.num_children() - 1));
    }
    else
    {
        assert(!subroot->is_leaf());
        auto& sr = subroot->as<internal_node>();
        sr.each_child(
            [&](const node* child)
            {
                debinarize_subtree(sr, bin_cat, child, res);
            });
    }
}
}
}
