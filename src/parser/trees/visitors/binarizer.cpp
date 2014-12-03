/**
 * @file binarizer.cpp
 * @author Chase Geigle
 */

#include <cassert>

#include "parser/trees/visitors/binarizer.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "util/shim.h"
#include "logging/logger.h"

namespace meta
{
namespace parser
{

namespace
{

/**
 * Visitor that sets the lexicons for each node in the tree recursively,
 * assuming that head constituents have been identified correctly. This is
 * needed since the position of the lexicon is not known until the
 * entire binarization is completely finished.
 */
struct lexicon_populator : public visitor<void>
{
    void operator()(leaf_node&) override
    {
        return;
    }

    void operator()(internal_node& inode) override
    {
        inode.each_child([&](node* child)
        {
            child->accept(*this);
        });

        if (!inode.head_lexicon())
            inode.head(inode.head_constituent());
    }
};

}

std::unique_ptr<node> binarizer::operator()(const leaf_node& ln)
{
    return make_unique<leaf_node>(ln);
}

std::unique_ptr<node> binarizer::operator()(const internal_node& in)
{
    auto res = make_unique<internal_node>(in.category());
    if (in.num_children() <= 2)
    {
        in.each_child(
            [&](const node* child)
            {
                auto nchild = child->accept(*this);
                auto nc = nchild.get();
                res->add_child(std::move(nchild));
                if (in.head_constituent() == child)
                    res->head(nc);
            });
        return std::move(res);
    }

    auto bin_lbl = class_label{static_cast<std::string>(in.category()) + "*"};
    lexicon_populator pop;

    // locate head node
    auto head = in.head_constituent();
    if (!head)
        throw exception{"Head constituent not labeled"};

    // binarize to the right of the head node
    auto curr = res.get();
    for (uint64_t idx = 0; idx < in.num_children(); ++idx)
    {
        assert(curr);
        auto ridx = in.num_children() - 1 - idx;
        auto end = in.child(ridx);

        // this is a special case for handling when the head is the very
        // first child: in this case, when we're one away from the head we
        // should just add the head and the current end child to the
        // current binary node and return (e.g., don't make things like
        // NP* -> NP)
        if (ridx == 1 && in.child(ridx - 1) == head)
        {
            curr->add_child(head->accept(*this));
            curr->add_child(end->accept(*this));
            curr->head_constituent(curr->child(0));
            res->accept(pop);
            return std::move(res);
        }

        if (end == head)
            break;

        auto bin = make_unique<internal_node>(bin_lbl);
        auto next = bin.get();

        curr->add_child(std::move(bin));
        curr->add_child(end->accept(*this));
        curr->head(curr->child(0));

        curr = next;
    }

    // binarize to the left of the head node
    for (uint64_t idx = 0; idx < in.num_children(); ++idx)
    {
        assert(curr);
        auto beg = in.child(idx);
        if (in.child(idx + 1) == head)
        {
            curr->add_child(beg->accept(*this));
            curr->add_child(head->accept(*this));
            curr->head(curr->child(1));
            break;
        }

        auto bin = make_unique<internal_node>(bin_lbl);
        auto next = bin.get();

        curr->add_child(beg->accept(*this));
        curr->add_child(std::move(bin));
        curr->head(curr->child(1));

        curr = next;
    }

    res->accept(pop);
    return std::move(res);
}
}
}
