/**
 * @file head_finder.cpp
 * @author Chase Geigle
 */

#include <unordered_set>
#include <vector>

#include "meta/logging/logger.h"
#include "meta/meta.h"
#include "meta/parser/trees/visitors/head_finder.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/util/optional.h"

namespace meta
{
namespace parser
{

namespace
{

/**
 * A normal head rule following Collins' head finding algorithm.
 */
struct normal_head_rule : public head_rule
{
    template <class... Args>
    normal_head_rule(Args&&... args)
        : candidates_({std::forward<Args>(args)...})
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

    uint64_t find_head(const internal_node& inode) const override
    {
        for (const auto& cand : candidates_)
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto child = inode.child(idx);
                if (child->category() == cand)
                    return idx;
            }
        }

        // no matches, use left most node
        return 0;
    }
};

/**
 * A head rule that starts its search from the rightmost child.
 */
struct head_final : public normal_head_rule
{
    using normal_head_rule::normal_head_rule;

    uint64_t find_head(const internal_node& inode) const override
    {
        for (const auto& cand : candidates_)
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                // iterate in reverse, from right to left
                auto ridx = inode.num_children() - 1 - idx;
                auto child = inode.child(ridx);
                if (child->category() == cand)
                    return ridx;
            }
        }

        // no matches, use right most node
        return inode.num_children() - 1;
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

        util::optional<uint64_t> find_head(const internal_node& inode) const
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto ridx = inode.num_children() - 1 - idx;
                auto child = inode.child(ridx);
                if (candidates_.find(child->category()) != candidates_.end())
                    return {ridx};
            }

            return util::nullopt;
        }
    };

    struct head_initial_np
    {
        std::unordered_set<class_label> candidates_;

        util::optional<uint64_t> find_head(const internal_node& inode) const
        {
            for (uint64_t idx = 0; idx < inode.num_children(); ++idx)
            {
                auto child = inode.child(idx);
                if (candidates_.find(child->category()) != candidates_.end())
                    return {idx};
            }

            return util::nullopt;
        }
    };

    uint64_t find_head(const internal_node& inode) const override
    {
        head_final_np first_pass{{"NN"_cl, "NNP"_cl, "NNPS"_cl, "NNS"_cl,
                                  "NX"_cl, "POS"_cl, "JJR"_cl}};
        if (auto idx = first_pass.find_head(inode))
            return *idx;

        head_initial_np second_pass{{"NP"_cl}};
        if (auto idx = second_pass.find_head(inode))
            return *idx;

        head_final_np third_pass{{"$"_cl, "ADJP"_cl, "PRN"_cl}};
        if (auto idx = third_pass.find_head(inode))
            return *idx;

        head_final_np fourth_pass{{"CD"_cl}};
        if (auto idx = fourth_pass.find_head(inode))
            return *idx;

        head_final_np fifth_pass{{"JJ"_cl, "JJS"_cl, "RB"_cl, "QP"_cl}};
        if (auto idx = fifth_pass.find_head(inode))
            return *idx;

        // no matches, just use last child
        return inode.num_children() - 1;
    }
};
}

head_finder::head_finder()
{
    /// @see: http://www.cs.columbia.edu/~mcollins/papers/heads
    /// @see Collins' thesis, page 240
    rules_["ADJP"_cl] = make_unique<head_initial>(
        "NNS"_cl, "QP"_cl, "NN"_cl, "$"_cl, "ADVP"_cl, "JJ"_cl, "VBN"_cl,
        "VBG"_cl, "ADJP"_cl, "JJR"_cl, "NP"_cl, "JJS"_cl, "DT"_cl, "FW"_cl,
        "RBR"_cl, "RBS"_cl, "SBAR"_cl, "RB"_cl);

    rules_["ADVP"_cl] = make_unique<head_final>(
        "RB"_cl, "RBR"_cl, "RBS"_cl, "FW"_cl, "ADVP"_cl, "TO"_cl, "CD"_cl,
        "JJR"_cl, "JJ"_cl, "IN"_cl, "NP"_cl, "JJS"_cl, "NN"_cl);

    rules_["CONJP"_cl] = make_unique<head_final>("CC"_cl, "RB"_cl, "IN"_cl);

    rules_["FRAG"_cl] = make_unique<head_final>();

    rules_["INTJ"_cl] = make_unique<head_initial>();

    rules_["LST"_cl] = make_unique<head_final>("LS"_cl, ":"_cl);

    rules_["NAC"_cl] = make_unique<head_initial>(
        "NN"_cl, "NNS"_cl, "NNP"_cl, "NNPS"_cl, "NP"_cl, "NAC"_cl, "EX"_cl,
        "$"_cl, "CD"_cl, "QP"_cl, "PRP"_cl, "VBG"_cl, "JJ"_cl, "JJS"_cl,
        "JJR"_cl, "ADJP"_cl, "FW"_cl);

    rules_["PP"_cl] = make_unique<head_final>("IN"_cl, "TO"_cl, "VBG"_cl,
                                              "VBN"_cl, "RP"_cl, "FW"_cl);

    rules_["PRN"_cl] = make_unique<head_initial>();

    rules_["PRT"_cl] = make_unique<head_final>("RP"_cl);

    rules_["QP"_cl] = make_unique<head_initial>(
        "$"_cl, "IN"_cl, "NNS"_cl, "NN"_cl, "JJ"_cl, "RB"_cl, "DT"_cl, "CD"_cl,
        "NCD"_cl, "QP"_cl, "JJR"_cl, "JJS"_cl);

    rules_["RRC"_cl] = make_unique<head_final>("VP"_cl, "NP"_cl, "ADVP"_cl,
                                               "ADJP"_cl, "PP"_cl);

    rules_["S"_cl]
        = make_unique<head_initial>("TO"_cl, "IN"_cl, "VP"_cl, "S"_cl,
                                    "SBAR"_cl, "ADJP"_cl, "UCP"_cl, "NP"_cl);

    rules_["SBAR"_cl] = make_unique<head_initial>(
        "WHNP"_cl, "WHPP"_cl, "WHADVP"_cl, "WHADJP"_cl, "IN"_cl, "DT"_cl,
        "S"_cl, "SQ"_cl, "SINV"_cl, "SBAR"_cl, "FRAG"_cl);

    rules_["SBARQ"_cl] = make_unique<head_initial>("SQ"_cl, "S"_cl, "SINV"_cl,
                                                   "SBARQ"_cl, "FRAG"_cl);

    rules_["SINV"_cl] = make_unique<head_initial>(
        "VBZ"_cl, "VBD"_cl, "VBP"_cl, "VB"_cl, "MD"_cl, "VP"_cl, "S"_cl,
        "SINV"_cl, "ADJP"_cl, "NP"_cl);

    rules_["SQ"_cl] = make_unique<head_initial>(
        "VBZ"_cl, "VBD"_cl, "VBP"_cl, "VB"_cl, "MD"_cl, "VP"_cl, "SQ"_cl);

    rules_["UCP"_cl] = make_unique<head_final>();

    rules_["VP"_cl] = make_unique<head_initial>(
        "TO"_cl, "VBD"_cl, "VBN"_cl, "MD"_cl, "VBZ"_cl, "VB"_cl, "VBG"_cl,
        "VBP"_cl, "VP"_cl, "ADJP"_cl, "NN"_cl, "NNS"_cl, "NP"_cl);

    rules_["WHADJP"_cl]
        = make_unique<head_initial>("CC"_cl, "WRB"_cl, "JJ"_cl, "ADJP"_cl);

    rules_["WHADVP"_cl] = make_unique<head_final>("CC"_cl, "WRB"_cl);

    rules_["WHNP"_cl] = make_unique<head_initial>(
        "WDT"_cl, "WP"_cl, "WP$"_cl, "WHADJP"_cl, "WHPP"_cl, "WHNP"_cl);

    rules_["WHPP"_cl] = make_unique<head_final>("IN"_cl, "TO"_cl, "FW"_cl);

    rules_["NP"_cl] = make_unique<head_np>();

    rules_["NX"_cl]
        = make_unique<head_initial>(); // not present in collins' thesis...

    rules_["X"_cl]
        = make_unique<head_final>(); // not present in collins' thesis...

    rules_["ROOT"_cl] = make_unique<head_initial>();
}

head_finder::head_finder(rule_table&& table) : rules_{std::move(table)}
{
    // nothing
}

void head_finder::operator()(leaf_node&)
{
    // head annotations are only populated for internal nodes; leaf nodes
    // are the trivial case
    return;
}

void head_finder::operator()(internal_node& inode)
{
    // recurse, as we need the head annotations of all child nodes first
    inode.each_child([&](node* child)
                     {
                         child->accept(*this);
                     });

    if (rules_.find(inode.category()) == rules_.end())
        LOG(fatal) << "No rule found for category " << inode.category()
                   << " in rule table" << ENDLG;

    // run the head finder for the syntactic category of the current node
    auto idx = rules_.at(inode.category())->find_head(inode);
    inode.head(inode.child(idx));

    if (idx < 2)
        return;

    static auto is_punctuation = [](const class_label& cat)
    {
        static std::unordered_set<class_label> punctuation = {
            "''"_cl, "``"_cl, "-LRB-"_cl, "-RRB-"_cl, "."_cl, ":"_cl, ";"_cl};

        return punctuation.find(cat) != punctuation.end();
    };

    // clean up stage for handling coordinating clauses
    if (inode.child(idx - 1)->category() == "CC"_cl
        || inode.child(idx - 1)->category() == "CONJP"_cl)
    {
        for (uint64_t i = 0; i <= idx - 2; ++i)
        {
            auto nidx = idx - 2 - i;
            auto child = inode.child(nidx);
            if (child->is_leaf() || !is_punctuation(child->category()))
            {
                inode.head(child);
                return;
            }
        }
    }
}
}
}
