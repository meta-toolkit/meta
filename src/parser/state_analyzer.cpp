/**
 * @file state_analyzer.cpp
 * @author Chase Geigle
 */

#include <cassert>

#include "meta/parser/state_analyzer.h"
#include "meta/parser/state.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

namespace
{
struct node_info
{
    std::string head_tag = "-NULL-";
    std::string head_word = "-NULL-";
    std::string category = "-NULL-";

    node_info(const node* n)
    {
        if (!n)
            return;

        category = n->category();
        if (n->is_leaf())
        {
            head_tag = n->category();
            head_word = *n->as<leaf_node>().word();
        }
        else
        {
            const auto& in = n->as<internal_node>();

            head_tag = in.head_lexicon()->category();
            head_word = *in.head_lexicon()->word();
        }
    }
};
}

auto sr_parser::state_analyzer::featurize(
    const state& state) const -> feature_vector
{
    feature_vector feats;

    unigram_featurize(state, feats);
    bigram_featurize(state, feats);
    trigram_featurize(state, feats);
    children_featurize(state, feats);
    dependents_featurize(state, feats);

    if (state.queue_size() == 0)
    {
        feats["queue_empty"] = 1;
        if (state.stack_size() == 1)
            feats["queue_empty_stack_single"] = 1;
    }

    return feats;
}

void sr_parser::state_analyzer::unigram_featurize(const state& state,
                                                  feature_vector& feats) const
{
    auto s0 = state.stack_item(0);
    unigram_stack_feats(s0, "s0", feats);

    auto s1 = state.stack_item(1);
    unigram_stack_feats(s1, "s1", feats);

    auto s2 = state.stack_item(2);
    unigram_stack_feats(s2, "s2", feats);

    auto s3 = state.stack_item(3);
    unigram_stack_feats(s3, "s3", feats);

    for (int64_t i = -2; i <= 3; ++i)
    {
        node_info info{state.queue_item(i)};
        feats["q" + std::to_string(i) + "wt=" + info.head_word + "-"
              + info.head_tag] = 1;
    }
}

void sr_parser::state_analyzer::unigram_stack_feats(const node* n,
                                                    std::string prefix,
                                                    feature_vector& feats) const
{
    node_info hi{n};

    feats[prefix + "c=" + hi.category] = 1;
    feats[prefix + "t=" + hi.head_tag] = 1;
    feats[prefix + "wc=" + hi.head_word + "-" + hi.category] = 1;
    feats[prefix + "wt=" + hi.head_word + "-" + hi.head_tag] = 1;
    feats[prefix + "tc=" + hi.head_tag + "-" + hi.category] = 1;
}

void sr_parser::state_analyzer::bigram_features(const node* n1,
                                                std::string name1,
                                                const node* n2,
                                                std::string name2,
                                                feature_vector& feats) const
{
    node_info n1h{n1};
    node_info n2h{n2};

    feats[name1 + "w" + name2 + "w=" + n1h.head_word + "-" + n2h.head_word] = 1;
    feats[name1 + "w" + name2 + "c=" + n1h.head_word + "-" + n2h.category] = 1;
    feats[name1 + "c" + name2 + "w=" + n1h.category + "-" + n2h.head_word] = 1;
    feats[name1 + "c" + name2 + "c=" + n1h.category + "-" + n2h.category] = 1;
}

void sr_parser::state_analyzer::bigram_featurize(const state& state,
                                                 feature_vector& feats) const
{
    bigram_features(state.stack_item(0), "s0", state.queue_item(0), "q0",
                    feats);
    bigram_features(state.stack_item(0), "s0", state.stack_item(1), "s1",
                    feats);
    bigram_features(state.stack_item(1), "s1", state.queue_item(0), "q0",
                    feats);
    bigram_features(state.queue_item(0), "q0", state.queue_item(1), "q1",
                    feats);
}

void sr_parser::state_analyzer::trigram_featurize(const state& state,
                                                  feature_vector& feats) const
{
    auto s0 = state.stack_item(0);
    auto s1 = state.stack_item(1);
    auto s2 = state.stack_item(2);
    auto q0 = state.queue_item(0);

    node_info s0h{s0};
    node_info s1h{s1};
    node_info s2h{s2};
    node_info q0h{q0};

    feats["s0cs1cs2c=" + s0h.category + "-" + s1h.category + "-" + s2h.category]
        = 1;

    feats["s0ws1cs2c=" + s0h.head_word + "-" + s1h.category + "-"
          + s2h.category] = 1;

    feats["s0cs1ws2c=" + s0h.category + "-" + s1h.head_word + "-"
          + s2h.category] = 1;

    feats["s0cs1cs2w=" + s0h.category + "-" + s1h.category + "-"
          + s2h.head_word] = 1;

    feats["s0cs1wq0t=" + s0h.category + "-" + s1h.head_word + "-"
          + q0h.head_tag] = 1;

    feats["s0cs1cq0t=" + s0h.category + "-" + s1h.category + "-" + q0h.head_tag]
        = 1;

    feats["s0ws1cq0t=" + s0h.head_word + "-" + s1h.category + "-"
          + q0h.head_tag] = 1;

    feats["s0cs1wq0t=" + s0h.category + "-" + s1h.head_word + "-"
          + q0h.head_tag] = 1;

    feats["s0cs1cq0w=" + s0h.category + "-" + s1h.category + "-"
          + q0h.head_word] = 1;
}

void sr_parser::state_analyzer::children_featurize(const state& state,
                                                   feature_vector& feats) const
{
    if (state.stack_size() > 0)
    {
        auto s0 = state.stack_item(0);
        child_feats(s0, "s0", feats, true);
    }

    if (state.stack_size() > 1)
    {
        auto s1 = state.stack_item(1);
        child_feats(s1, "s1", feats, true);
    }
}

void sr_parser::state_analyzer::child_feats(const node* n, std::string prefix,
                                            feature_vector& feats,
                                            bool doubs) const
{
    if (n->is_leaf())
        return;

    const auto& in = n->as<internal_node>();

    assert(in.num_children() <= 2);

    if (in.num_children() == 2)
    {
        unigram_stack_feats(in.child(0), prefix + "l", feats);
        unigram_stack_feats(in.child(1), prefix + "r", feats);

        if (doubs)
        {
            child_feats(in.child(0), prefix + "l", feats, false);
            child_feats(in.child(1), prefix + "r", feats, false);
        }
    }
    else
    {
        assert(in.num_children() == 1);

        unigram_stack_feats(in.child(0), prefix + "u", feats);

        // TODO: better condition for this?
        if (doubs && prefix == "s0")
            child_feats(in.child(0), prefix + "u", feats, false);
    }
}

namespace
{
const node* left_dependent(const node* n)
{
    if (n)
    {
        node_info hi{n};

        while (!n->is_leaf())
        {
            const auto& in = n->as<internal_node>();
            auto child = in.child(0);
            node_info chi{child};

            if (chi.head_word != hi.head_word)
                return child;

            n = child;
        }
    }

    return nullptr;
}

const node* right_dependent(const node* n)
{
    if (n)
    {
        node_info hi{n};

        while (!n->is_leaf())
        {
            const auto& in = n->as<internal_node>();
            if (in.num_children() == 1)
            {
                n = in.child(0);
            }
            else
            {
                assert(in.num_children() == 2);

                auto child = in.child(1);
                node_info chi{child};

                if (chi.head_word != hi.head_word)
                    return child;

                n = child;
            }
        }
    }

    return nullptr;
}
}

void
    sr_parser::state_analyzer::dependents_featurize(const state& state,
                                                    feature_vector& feats) const
{
    unigram_stack_feats(left_dependent(state.stack_item(0)), "rs0l", feats);
    unigram_stack_feats(left_dependent(state.stack_item(1)), "rs1l", feats);
    unigram_stack_feats(right_dependent(state.stack_item(0)), "rs0r", feats);
    unigram_stack_feats(right_dependent(state.stack_item(1)), "rs1r", feats);
}
}
}
