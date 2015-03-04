/**
 * @file transition_finder.cpp
 * @author Chase Geigle
 */

#include "parser/trees/visitors/transition_finder.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

void transition_finder::operator()(const leaf_node&)
{
    transitions_.emplace_back(transition::type_t::SHIFT);
}

void transition_finder::operator()(const internal_node& in)
{
    if (in.num_children() > 2)
        throw exception{
            "Trees must be binarized before transitions are generated"};

    in.each_child([&](const node* n)
                  {
                      n->accept(*this);
                  });

    if (in.num_children() == 1)
    {
        transitions_.emplace_back(transition::type_t::UNARY, in.category());
    }
    else if (in.child(0) == in.head_constituent())
    {
        transitions_.emplace_back(transition::type_t::REDUCE_L, in.category());
    }
    else if (in.child(1) == in.head_constituent())
    {
        transitions_.emplace_back(transition::type_t::REDUCE_R, in.category());
    }
    else
    {
        throw exception{"Incorrect head annotations (head was neither left nor "
                        "right child)"};
    }
}

std::vector<transition> transition_finder::transitions()
{
    transitions_.emplace_back(transition::type_t::FINALIZE);
    transitions_.emplace_back(transition::type_t::IDLE);
    return std::move(transitions_);
}
}
}
