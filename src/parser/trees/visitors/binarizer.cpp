/**
 * @file binarizer.cpp
 * @author Chase Geigle
 */

#include <cassert>

#include "meta/parser/trees/visitors/binarizer.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/shim.h"
#include "meta/logging/logger.h"

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
        in.each_child([&](const node* child)
                      {
                          res->add_child(child->accept(*this));
                          if (child == in.head_constituent())
                              res->head(res->child(res->num_children() - 1));
                      });

        return std::move(res);
    }

    auto bin_lbl = class_label{static_cast<std::string>(in.category()) + "*"};

    // locate head node
    auto head = in.head_constituent();
    if (!head)
        throw tree_binarizer_exception{"Head constituent not labeled"};

    uint64_t head_idx = 0;
    for (uint64_t idx = 0; idx < in.num_children(); ++idx)
    {
        if (in.child(idx) == in.head_constituent())
            head_idx = idx;
    }

    // eat left nodes
    auto curr = res.get();
    for (uint64_t idx = 0; idx < head_idx; ++idx)
    {
        curr->add_child(in.child(idx)->accept(*this));

        // special case: if the head is the very last node, just add the
        // remaining child (the head) to the current node
        if (idx + 1 == head_idx && head_idx == in.num_children() - 1)
        {
            curr->add_child(in.child(idx + 1)->accept(*this));
            curr->head(curr->child(1));
            break;
        }
        else
        {
            auto bin = make_unique<internal_node>(bin_lbl);
            auto next = bin.get();

            curr->add_child(std::move(bin));
            curr->head(curr->child(1));

            curr = next;
        }
    }

    // eat right nodes
    for (uint64_t ridx = in.num_children() - 1; ridx > head_idx; --ridx)
    {
        // if the head is the next node, just add it and the current child
        if (head_idx + 1 == ridx)
        {
            curr->add_child(in.child(ridx - 1)->accept(*this));
            curr->head(curr->child(0));
            curr->add_child(in.child(ridx)->accept(*this));
            break;
        }
        else
        {
            auto bin = make_unique<internal_node>(bin_lbl);
            auto next = bin.get();

            curr->add_child(std::move(bin));
            curr->head(curr->child(0));
            curr->add_child(in.child(ridx)->accept(*this));

            curr = next;
        }
    }

    lexicon_populator pop;
    res->accept(pop);
    return std::move(res);
}
}
}
