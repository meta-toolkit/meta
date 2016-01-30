/**
 * @file evalb.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <set>
#include <unordered_set>
#include "meta/util/comparable.h"
#include "meta/parser/trees/evalb.h"
#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/visitors/annotation_remover.h"
#include "meta/parser/trees/visitors/tree_transformer.h"
#include "meta/parser/trees/visitors/empty_remover.h"
#include "meta/parser/trees/visitors/multi_transformer.h"
#include "meta/parser/trees/visitors/unary_chain_remover.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

namespace
{
struct constituent : public util::comparable<constituent>
{
    class_label category;
    uint64_t start;
    uint64_t end; // non-inclusive!
};

bool crosses(const constituent& first, const constituent& second)
{
    return (first.start < second.start && second.start < first.end
            && first.end < second.end)
           || (second.start < first.start && first.start < second.end
               && second.end < first.end);
}

bool operator<(const constituent& first, const constituent& second)
{
    return std::tie(first.category, first.start, first.end)
           < std::tie(second.category, second.start, second.end);
}

class constituent_finder : public const_visitor<void>
{
  public:
    void operator()(const leaf_node&) override
    {
        ++curr_leaf_;
    }

    void operator()(const internal_node& in) override
    {
        constituent con;
        con.category = in.category();
        con.start = curr_leaf_;

        in.each_child([&](const node* child)
                      {
                          child->accept(*this);
                      });
        con.end = curr_leaf_;

        constituents_.emplace(std::move(con));
    }

    std::multiset<constituent> constituents()
    {
        return std::move(constituents_);
    }

  private:
    uint64_t curr_leaf_ = 0;
    std::multiset<constituent> constituents_;
};

std::multiset<constituent> get_constituents(const parse_tree& tree)
{
    constituent_finder const_finder;
    tree.visit(const_finder);
    return const_finder.constituents();
}

class collinizer : public tree_transformer
{
  public:
    std::unique_ptr<node> operator()(const leaf_node& ln) override
    {
        // remove punctuation preterminals
        if (punct_cats.find(ln.category()) != punct_cats.end())
            return nullptr;

        return ln.clone();
    }

    std::unique_ptr<node> operator()(const internal_node& in) override
    {
        std::unique_ptr<internal_node> res;

        // remove the top root node
        if (in.category() == "ROOT"_cl)
            return in.child(0)->accept(*this);

        // weird equivalence...
        if (in.category() == "PRT"_cl)
            res = make_unique<internal_node>("ADVP"_cl);
        else
            res = make_unique<internal_node>(in.category());

        in.each_child([&](const node* c)
                      {
                          auto child = c->accept(*this);
                          if (child)
                              res->add_child(std::move(child));
                      });

        return std::move(res);
    }

  private:
    const std::unordered_set<class_label> punct_cats
        = {"''"_cl, "'"_cl, "``"_cl, "``"_cl, "."_cl, ":"_cl, ","_cl};
};
}

void evalb::add_tree(parse_tree proposed, parse_tree gold)
{

    static multi_transformer<annotation_remover, collinizer, empty_remover>
        trnsfm;

    proposed.transform(trnsfm);
    gold.transform(trnsfm);

    auto prop_const = get_constituents(proposed);
    auto gold_const = get_constituents(gold);

    uint64_t crossings = 0;
    for (const auto& guess : prop_const)
    {
        for (const auto& gold : gold_const)
        {
            if (crosses(guess, gold))
            {
                ++crossings;
                break;
            }
        }
    }
    if (crossings == 0)
        ++zero_crossing_;
    crossed_ += crossings;
    ++total_trees_;

    proposed_total_ += prop_const.size();
    uint64_t gold_total = gold_const.size();
    uint64_t matched = 0;
    for (const auto& guess : prop_const)
    {
        auto it = gold_const.find(guess);
        if (it != gold_const.end())
        {
            ++matched;
            gold_const.erase(it);
        }
    }

    if (matched == gold_total && matched == prop_const.size())
        ++perfect_;

    gold_total_ += gold_total;
    proposed_correct_ += matched;
}
}
}
