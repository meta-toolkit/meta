/**
 * @file leaf_node_finder.cpp
 * @author Chase Geigle
 */

#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "parser/trees/visitors/leaf_node_finder.h"
#include "util/shim.h"

namespace meta
{
namespace parser
{

void leaf_node_finder::operator()(const leaf_node& ln)
{
    leaves_.push_back(make_unique<leaf_node>(ln));
}

void leaf_node_finder::operator()(const internal_node& in)
{
    in.each_child([&](const node* n)
                  {
                      n->accept(*this);
                  });
}

std::vector<std::unique_ptr<leaf_node>> leaf_node_finder::leaves()
{
    return std::move(leaves_);
}
}
}
