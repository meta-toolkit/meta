/**
 * @file head_finder.cpp
 * @author Chase Geigle
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "meta.h"
#include "parser/trees/visitors/head_finder.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

namespace
{

void set_head(internal_node& inode, const node* child)
{
    inode.head_constituent(child);

    if (child->is_leaf())
        inode.head_lexicon(&child->as<leaf_node>());
    else
        inode.head_lexicon(child->as<internal_node>().head_lexicon());
}

bool handle_child(internal_node& inode, const node* child,
                  const class_label& cand)
{
    if (child->category() == cand)
    {
        set_head(inode, child);
        return true;
    }
    return false;
}

/**
 * A normal head rule following Collins' head finding algorithm.
 */
struct normal_head_rule : public head_rule
{
    template <class... Args>
    normal_head_rule(Args&&... args)
        : candidates_{{std::forward<Args>(args)...}}
    {
        // nothing
    }

    std::vector<class_label> candidates_;
};

/**
 * A head rule that starts its search from the leftmost child.
 */
struct head_initial : public normal_head_rule
{
    using normal_head_rule::normal_head_rule;

    void find_head(internal_node& inode) const override
    {
        for (const auto& cand : candidates_)
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto child = inode.child(idx);
                if (handle_child(inode, child, cand))
                    return;
            }
        }

        // no matches, use left most node
        auto child = inode.child(0);
        set_head(inode, child);
    }
};

/**
 * A head rule that starts its search from the rightmost child.
 */
struct head_final : public normal_head_rule
{
    using normal_head_rule::normal_head_rule;

    void find_head(internal_node& inode) const override
    {
        for (const auto& cand : candidates_)
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                // iterate in reverse, from right to left
                auto ridx = inode.num_children() - 1 - idx;
                auto child = inode.child(ridx);
                if (handle_child(inode, child, cand))
                    return;
            }
        }

        // no matches, use right most node
        auto child = inode.child(inode.num_children() - 1);
        set_head(inode, child);
    }
};

/**
 * The special case for noun phrases in Collins' head finding algorithm.
 * @see Collins' thesis, page 238-239
 */
struct head_np : public head_rule
{
    struct head_final_np
    {
        std::unordered_set<class_label> candidates_;

        bool find_head(internal_node& inode) const
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto ridx = inode.num_children() - 1 - idx;
                auto child = inode.child(ridx);
                if (candidates_.find(child->category()) != candidates_.end())
                {
                    set_head(inode, child);
                    return true;
                }
            }

            return false;
        }
    };

    struct head_initial_np
    {
        std::unordered_set<class_label> candidates_;

        bool find_head(internal_node& inode) const
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto child = inode.child(idx);
                if (candidates_.find(child->category()) != candidates_.end())
                {
                    set_head(inode, child);
                    return true;
                }
            }

            return false;
        }
    };

    void find_head(internal_node& inode) const override
    {
        head_final_np first_pass{
            {"NN", "NNP", "NNPS", "NNS", "NX", "POS", "JJR"}};
        if (first_pass.find_head(inode))
            return;

        head_initial_np second_pass{{"NP"}};
        if (second_pass.find_head(inode))
            return;

        head_final_np third_pass{{"$", "ADJP", "PRN"}};
        if (third_pass.find_head(inode))
            return;

        head_final_np fourth_pass{{"CD"}};
        if (fourth_pass.find_head(inode))
            return;

        head_final_np fifth_pass{{"JJ", "JJS", "RB", "QP"}};
        if (fifth_pass.find_head(inode))
            return;

        // no matches, just use last child
        auto child = inode.child(inode.num_children() - 1);
        set_head(inode, child);
    }
};
}

head_finder::head_finder()
{
    /// @see: http://www.cs.columbia.edu/~mcollins/papers/heads
    /// @see Collins' thesis, page 240
    rules_["ADJP"] = make_unique<head_initial>(
        "NNS", "QP", "NN", "$", "ADVP", "JJ", "VBN", "VBG", "ADJP", "JJR", "NP",
        "JJS", "DT", "FW", "RBR", "RBS", "SBAR", "RB");

    rules_["ADVP"]
        = make_unique<head_final>("RB", "RBR", "RBS", "FW", "ADVP", "TO", "CD",
                                  "JJR", "JJ", "IN", "NP", "JJS", "NN");

    rules_["CONJP"] = make_unique<head_final>("CC", "RB", "IN");

    rules_["FRAG"] = make_unique<head_final>();

    rules_["INTJ"] = make_unique<head_initial>();

    rules_["LST"] = make_unique<head_final>("LS", ":");

    rules_["NAC"] = make_unique<head_initial>(
        "NN", "NNS", "NNP", "NNPS", "NP", "NAC", "EX", "$", "CD", "QP", "PRP",
        "VBG", "JJ", "JJS", "JJR", "ADJP", "FW");

    rules_["PP"]
        = make_unique<head_final>("IN", "TO", "VBG", "VBN", "RP", "FW");

    rules_["PRN"] = make_unique<head_initial>();

    rules_["PRT"] = make_unique<head_final>("RP");

    rules_["QP"]
        = make_unique<head_initial>("$", "IN", "NNS", "NN", "JJ", "RB", "DT",
                                    "CD", "NCD", "QP", "JJR", "JJS");

    rules_["RRC"] = make_unique<head_final>("VP", "NP", "ADVP", "ADJP", "PP");

    rules_["S"] = make_unique<head_initial>("TO", "IN", "VP", "S", "SBAR",
                                            "ADJP", "UCP", "NP");

    rules_["SBAR"]
        = make_unique<head_initial>("WHNP", "WHPP", "WHADVP", "WHADJP", "IN",
                                    "DT", "S", "SQ", "SINV", "SBAR", "FRAG");

    rules_["SBARQ"]
        = make_unique<head_initial>("SQ", "S", "SINV", "SBARQ", "FRAG");

    rules_["SINV"] = make_unique<head_initial>("VBZ", "VBD", "VBP", "VB", "MD",
                                               "VP", "S", "SINV", "ADJP", "NP");

    rules_["SQ"] = make_unique<head_initial>("VBZ", "VBD", "VBP", "VB", "MD",
                                             "VP", "SQ");

    rules_["UCP"] = make_unique<head_final>();

    rules_["VP"] = make_unique<head_initial>("TO", "VBD", "VBN", "MD", "VBZ",
                                             "VB", "VBG", "VBP", "VP", "ADJP",
                                             "NN", "NNS", "NP");

    rules_["WHADJP"] = make_unique<head_initial>("CC", "WRB", "JJ", "ADJP");

    rules_["WHADVP"] = make_unique<head_final>("CC", "WRB");

    rules_["WHNP"] = make_unique<head_initial>("WDT", "WP", "WP$", "WHADJP",
                                               "WHPP", "WHNP");

    rules_["WHPP"] = make_unique<head_final>("IN", "TO", "FW");

    rules_["NP"] = make_unique<head_np>();
}

head_finder::head_finder(rule_table&& table) : rules_{std::move(table)}
{
    // nothing
}
}
}
