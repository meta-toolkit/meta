/**
 * @file state.cpp
 * @author Chase Geigle
 */

#include "parser/trees/visitors/leaf_node_finder.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "parser/state.h"

namespace meta
{
namespace parser
{

state::state(const parse_tree& tree)
{
    leaf_node_finder lnf;
    tree.visit(lnf);
    queue_ = lnf.leaves();
    q_idx_ = 0;
    done_ = false;
}

void state::advance(const transition& trans)
{
    switch (trans.type())
    {
        case transition::type_t::SHIFT:
        {
            stack_ = stack_.push(queue_[q_idx_]->clone());
            ++q_idx_;
        }
        break;

        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
        {
            auto right = stack_.peek()->clone();
            stack_ = stack_.pop();
            auto left = stack_.peek()->clone();
            stack_ = stack_.pop();

            auto bin = make_unique<internal_node>(trans.label());
            bin->add_child(std::move(left));
            bin->add_child(std::move(right));

            if (trans.type() == transition::type_t::REDUCE_L)
            {
                bin->head(bin->child(0));
            }
            else
            {
                bin->head(bin->child(1));
            }

            stack_ = stack_.push(std::move(bin));
        }
        break;

        case transition::type_t::UNARY:
        {
            auto child = stack_.peek()->clone();
            stack_ = stack_.pop();

            auto un = make_unique<internal_node>(trans.label());
            un->add_child(std::move(child));
            un->head(un->child(0));

            stack_ = stack_.push(std::move(un));
        }
        break;

        case transition::type_t::FINALIZE:
        {
            done_ = true;
        }
        break;

        case transition::type_t::IDLE:
        {
            // nothing
        }
        break;
    }
}

namespace
{

bool is_temporary(const std::string& lbl)
{
    return lbl[lbl.length() - 1] == '*';
}

bool is_temporary(const node* n)
{
    if (n->is_leaf())
        return false;

    const auto& lbl = n->category();
    return is_temporary(lbl);
}

bool shift_legal(const state& state)
{
    // can't shift if there are no preterminals left on the queue
    if (state.queue_size() == 0)
        return false;

    // From Zhang and Clark (2009): when the node on the top of the stack
    // is a temporary node and its head word is from the right, no shift is
    // allowed
    if (state.stack_size() > 0)
    {
        auto top = state.stack_item(0);
        if (is_temporary(top))
        {
            const auto& in = top->as<internal_node>();
            if (in.num_children() == 2 && in.head_constituent() == in.child(1))
                return false;
        }
    }

    return true;
}

bool unary_legal(const state& state)
{
    if (state.stack_size() == 0)
        return false;

    // From Zhang and Clark (2009): no more than three unary reduce actions
    // can be performed consecutively
    if (!state.stack_item(0)->is_leaf())
    {
        const auto& in = state.stack_item(0)->as<internal_node>();
        if (in.num_children() > 1 || in.child(0)->is_leaf())
            return true;

        const auto& in2 = in.child(0)->as<internal_node>();

        if (in2.num_children() > 1 || in.child(0)->is_leaf())
            return true;

        const auto& in3 = in2.child(0)->as<internal_node>();
        if (in3.num_children() > 1)
            return true;
        else
            return false;
    }

    return true;
}

bool reduce_legal(const state& state, const transition& trans)
{
    // These restrictions are from Zhang and Clark (2009); text lifted from
    // their appendix for comments

    if (state.stack_size() < 2)
        return false;

    auto left = state.stack_item(1);
    auto right = state.stack_item(0);

    // binary reduce actions can only be performed with at least one of the
    // two nodes at the top of the stack being non-temporary
    if (is_temporary(left) && is_temporary(right))
        return false;

    // if L is temporary with label X*, the resulting node must be labeled
    // X or X* and left-headed
    if (is_temporary(left))
    {
        if (trans.type() == transition::type_t::REDUCE_R)
            return false;

        const std::string& lc = left->category();
        auto lbl = lc.substr(0, lc.length() - 1);
        const std::string& trans_lbl = trans.label();
        if (trans_lbl.find(lbl) == std::string::npos)
            return false;
    }

    // if R is a temporary with label X*, the resulting node must be
    // labeled X or X* and be right-headed
    if (is_temporary(right))
    {
        if (trans.type() == transition::type_t::REDUCE_L)
            return false;

        const std::string& rc = right->category();
        auto lbl = rc.substr(0, rc.length() - 1);
        const std::string& trans_lbl = trans.label();
        if (trans_lbl.find(lbl) == std::string::npos)
            return false;
    }

    // if the queue is empty and the stack contains only two nodes, binary
    // reduce can only be applied if the resulting node is non-temporary
    if (state.queue_size() == 0 && state.stack_size() == 2
        && is_temporary(trans.label()))
        return false;

    // if the stack contains only two nodes, temporary resulting nodes
    // from binary reduce must be left-headed
    if (state.stack_size() == 2 && trans.type() == transition::type_t::REDUCE_R)
        return false;

    if (state.stack_size() > 2 && is_temporary(state.stack_item(2)))
    {
        // if the queue is empty and the stack contains more than two nodes,
        // with the third node from the top being temporary, binary reduce can
        // only be applied if the resulting node is non temporary
        if (state.queue_size() == 0 && is_temporary(trans.label()))
            return false;

        // if the stack contains more than two nodes, with the third node
        // from the top being temporary, temporary resulting nodes from
        // binary reduce must be left headed
        if (is_temporary(trans.label())
            && trans.type() == transition::type_t::REDUCE_R)
            return false;
    }

    return true;
}

bool finalize_legal(const state& state)
{
    return state.queue_size() == 0 && state.stack_size() == 1;
}

bool idle_legal(const state& state)
{
    return state.finalized();
}
}

bool state::legal(const transition& trans) const
{
    // this is sort of ugly, but I think the ugliness introduced into the
    // sr_parser class by inducing an inheritance hierarchy of transitions
    // outweighs this class's ugly methods
    switch (trans.type())
    {
        case transition::type_t::SHIFT:
            return shift_legal(*this);

        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
            return reduce_legal(*this, trans);

        case transition::type_t::UNARY:
            return unary_legal(*this);

        case transition::type_t::FINALIZE:
            return finalize_legal(*this);

        case transition::type_t::IDLE:
            return idle_legal(*this);
    }
}

uint64_t state::stack_size() const
{
    return stack_.size();
}

uint64_t state::queue_size() const
{
    return queue_.size() - q_idx_;
}

const node* state::stack_item(size_t depth) const
{
    auto st = stack_;
    for (uint64_t i = 0; i < depth; ++i)
        st = st.pop();
    return st.peek().get();
}

const leaf_node* state::queue_item(size_t depth) const
{
    return queue_.at(q_idx_ + depth).get();
}

bool state::finalized() const
{
    return done_;
}
}
}
