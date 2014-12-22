/**
 * @file state_analyzer.cpp
 * @author Chase Geigle
 */

#include <cassert>

#include "parser/state_analyzer.h"
#include "parser/state.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{

namespace
{
struct head_info
{
    std::string head_tag;
    std::string head_word;

    head_info(const node* n)
    {
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

    return feats;
}

void sr_parser::state_analyzer::unigram_featurize(const state& state,
                                                  feature_vector& feats) const
{
    if (state.stack_size() > 0)
    {
        auto s0 = state.stack_item(0);
        unigram_stack_feats(s0, "s0", feats);
    }

    if (state.stack_size() > 1)
    {
        auto s1 = state.stack_item(1);
        unigram_stack_feats(s1, "s1", feats);
    }

    if (state.stack_size() > 2)
    {
        auto s2 = state.stack_item(2);
        unigram_stack_feats(s2, "s2", feats);
    }

    if (state.stack_size() > 3)
    {
        auto s3 = state.stack_item(3);
        unigram_stack_feats(s3, "s3", feats);
    }

    for (int i = 0; i <= 3 && state.queue_size() - i > 0; ++i)
    {
        auto word = *state.queue_item(i)->word();
        const std::string& tag = state.queue_item(i)->category();
        feats["q" + std::to_string(i) + "wt=" + word + "-" + tag] = 1;
    }
}

void sr_parser::state_analyzer::unigram_stack_feats(const node* n,
                                                    std::string prefix,
                                                    feature_vector& feats) const
{
    head_info hi{n};

    feats[prefix + "c=" + (std::string)n->category()] = 1;
    feats[prefix + "t=" + hi.head_tag] = 1;
    feats[prefix + "wc=" + hi.head_word + "-" + (std::string)n->category()] = 1;
    feats[prefix + "wt=" + hi.head_word + "-" + hi.head_tag] = 1;
    feats[prefix + "tc=" + hi.head_tag + "-" + (std::string)n->category()] = 1;
}

void sr_parser::state_analyzer::bigram_featurize(const state& state,
                                                 feature_vector& feats) const
{
    if (state.stack_size() > 0 && state.queue_size() > 0)
    {
        // can access s0 and q0
        auto s0 = state.stack_item(0);
        auto q0 = state.queue_item(0);

        head_info s0h{s0};
        head_info q0h{q0};

        feats["s0wq0w=" + s0h.head_word + "-" + q0h.head_word] = 1;
        feats["s0wq0t=" + s0h.head_word + "-" + q0h.head_tag] = 1;
        feats["s0cq0w=" + (std::string)s0->category() + "-" + q0h.head_word]
            = 1;
        feats["s0cq0t=" + (std::string)s0->category() + "-" + q0h.head_tag] = 1;

        if (state.stack_size() > 1)
        {
            // can access s1
            auto s1 = state.stack_item(1);

            head_info s1h{s1};

            feats["s0ws1w=" + s0h.head_word + "-" + s1h.head_word] = 1;
            feats["s0ws1c=" + s0h.head_word + "-" + (std::string)s1->category()]
                = 1;
            feats["s0cs1w=" + (std::string)s0->category() + "-" + s1h.head_word]
                = 1;
            feats["s0cs1c=" + (std::string)s0->category() + "-"
                  + (std::string)s1->category()] = 1;

            feats["s1wq0w=" + s1h.head_word + "-" + q0h.head_word] = 1;
            feats["s1wq0t=" + s1h.head_word + "-" + q0h.head_tag] = 1;
            feats["s1cq0w=" + (std::string)s1->category() + "-" + q0h.head_word]
                = 1;
            feats["s1cq0t=" + (std::string)s1->category() + "-" + q0h.head_tag]
                = 1;
        }

        if (state.queue_size() > 1)
        {
            // can access q1
            auto q1 = state.queue_item(1);

            head_info q1h{q1};

            feats["q0wq1w=" + q0h.head_word + "-" + q1h.head_word] = 1;
            feats["q0wq1t=" + q0h.head_word + "-" + q1h.head_tag] = 1;
            feats["q0tq1w=" + q0h.head_tag + "-" + q1h.head_word] = 1;
            feats["q0tq1t=" + q0h.head_tag + "-" + q1h.head_tag] = 1;
        }
    }
}

void sr_parser::state_analyzer::trigram_featurize(const state& state,
                                                  feature_vector& feats) const
{
    if (state.stack_size() < 2)
        return;

    auto s0 = state.stack_item(0);
    auto s1 = state.stack_item(1);

    head_info s0h{s0};
    head_info s1h{s1};

    if (state.stack_size() > 2)
    {
        // can access s2
        auto s2 = state.stack_item(2);

        head_info s2h{s2};

        feats["s0cs1cs2c=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + (std::string)s2->category()]
            = 1;

        feats["s0ws1cs2c=" + s0h.head_word + "-" + (std::string)s1->category()
              + "-" + (std::string)s2->category()] = 1;

        feats["s0cs1cs2w=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + s2h.head_word] = 1;
    }

    if (state.queue_size() > 0)
    {
        // can access q0
        auto q0 = state.queue_item(0);

        head_info q0h{q0};

        feats["s0cs1wq0t=" + (std::string)s0->category() + "-" + s1h.head_word
              + "-" + q0h.head_tag] = 1;

        feats["s0cs1cq0t=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + q0h.head_tag] = 1;

        feats["s0ws1cq0t=" + s0h.head_word + "-" + (std::string)s1->category()
              + "-" + q0h.head_tag] = 1;

        feats["s0cs1wq0t=" + (std::string)s0->category() + "-" + s1h.head_word
              + "-" + q0h.head_tag] = 1;

        feats["s0cs1cq0w=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + q0h.head_word] = 1;
    }
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
}
}
