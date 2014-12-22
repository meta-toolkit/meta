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

sr_parser::state::state(const parse_tree& tree)
{
    leaf_node_finder lnf;
    tree.visit(lnf);
    queue_ = lnf.leaves();
    q_idx_ = 0;
    done_ = false;
}

void sr_parser::state::advance(transition trans)
{
    // TODO: Inheritance hierarchy?
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

uint64_t sr_parser::state::stack_size() const
{
    return stack_.size();
}

uint64_t sr_parser::state::queue_size() const
{
    return queue_.size() - q_idx_;
}

const node* sr_parser::state::stack_item(size_t depth) const
{
    auto st = stack_;
    for (uint64_t i = 0; i < depth; ++i)
        st = st.pop();
    return st.peek().get();
}

const leaf_node* sr_parser::state::queue_item(size_t depth) const
{
    return queue_.at(q_idx_ + depth).get();
}

}
}
