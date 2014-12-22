/**
 * @file sr_parser.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <fstream>
#include <numeric>

#include "io/binary.h"
#include "logging/logger.h"
#include "parallel/parallel_for.h"
#include "parser/sr_parser.h"
#include "parser/state.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"
#include "parser/trees/visitors/multi_transformer.h"
#include "parser/trees/visitors/head_finder.h"
#include "parser/trees/visitors/binarizer.h"
#include "parser/trees/visitors/debinarizer.h"
#include "parser/trees/visitors/transition_finder.h"
#include "parser/trees/visitors/leaf_node_finder.h"
#include "util/progress.h"
#include "util/range.h"

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

sr_parser::training_data::training_data(training_options opt,
                                        std::vector<parse_tree>& trs)
    : options(opt), trees(trs), indices(trs.size()), rng{opt.seed}
{
    std::iota(indices.begin(), indices.end(), 0ul);
}

auto sr_parser::training_data::preprocess() -> transition_map
{
    transition_map trans_map;

    multi_transformer<annotation_remover, empty_remover, unary_chain_remover>
        transformer;

    head_finder hf;
    binarizer bin;

    printing::progress progress{" > Preprocessing training trees: ",
                                trees.size()};
    size_t idx = 0;
    for (auto& tree : trees)
    {
        progress(++idx);
        tree.transform(transformer);
        tree.visit(hf);
        tree.transform(bin);

        transition_finder trans;
        tree.visit(trans);

        auto transitions = trans.transitions();
        std::vector<trans_id> tids;
        tids.reserve(transitions.size());
        for (const auto& trans : transitions)
            tids.push_back(trans_map[trans]);
        all_transitions.emplace_back(std::move(tids));
    }

    return trans_map;
}

void sr_parser::training_data::shuffle()
{
    std::shuffle(indices.begin(), indices.end(), rng);
}

size_t sr_parser::training_data::size() const
{
    return indices.size();
}

const parse_tree& sr_parser::training_data::tree(size_t idx) const
{
    return trees[indices[idx]];
}

auto sr_parser::training_data::transitions(
    size_t idx) const -> const std::vector<trans_id> &
{
    return all_transitions[indices[idx]];
}

const transition& sr_parser::transition_map::at(trans_id id) const
{
    return transitions_.at(id);
}

auto sr_parser::transition_map::at(const transition& trans) const -> trans_id
{
    auto it = map_.find(trans);
    if (it == map_.end())
        throw std::out_of_range{"index out of bounds"};

    return it->second;
}

transition& sr_parser::transition_map::operator[](trans_id id)
{
    return transitions_.at(id);
}

auto sr_parser::transition_map::operator[](const transition& trans) -> trans_id
{
    auto it = map_.find(trans);
    if (it != map_.end())
        return it->second;

    transitions_.push_back(trans);
    auto id = static_cast<trans_id>(map_.size());
    return map_[trans] = id;
}

uint64_t sr_parser::transition_map::size() const
{
    assert(map_.size() == transitions_.size());
    return map_.size();
}

sr_parser::sr_parser(const std::string& prefix) : trans_{prefix}
{
    load(prefix);
}

void sr_parser::train(std::vector<parse_tree>& trees, training_options options)
{
    training_data data{options, trees};
    trans_ = data.preprocess();

    LOG(info) << "Found " << trans_.size() << " transitions" << ENDLG;

    parallel::thread_pool pool{options.num_threads};

    for (uint64_t iter = 1; iter <= options.max_iterations; ++iter)
    {
        printing::progress progress{
            " > Iteration " + std::to_string(iter) + ": ", trees.size()};
        data.shuffle();

        for (size_t start = 0; start < data.size(); start += options.batch_size)
        {
            progress(start);
            auto end = std::min(start + options.batch_size, data.size());

            auto update = train_batch({data, start, end}, pool);

            for (const auto& feat_vec : update)
            {
                const auto& feat = feat_vec.first;
                auto& wv = weights_[feat];
                for (const auto& up : feat_vec.second)
                    wv[up.first] += up.second;
            }
        }
        progress.end();

        condense();
    }
}

auto sr_parser::train_batch(training_batch batch, parallel::thread_pool& pool)
    -> weight_vectors
{
    // TODO: real beam search

    weight_vectors update;
    auto range = util::range(batch.start, batch.end - 1); // inclusive range

    // Perform a reduction across threads: each thread stores its update in
    // a separate location, and we then add them all up after we join
    util::sparse_vector<std::thread::id, weight_vectors> updates;
    for (const auto& tid : pool.thread_ids())
        updates[tid] = {};

    parallel::parallel_for(range.begin(), range.end(), pool, [&](size_t i)
                           {
        auto& tree = batch.data.tree(i);
        auto& transitions = batch.data.transitions(i);

        state state{tree};

        for (const auto& gold_trans : transitions)
        {
            auto feats = featurize(state);
            auto trans = best_transition(feats);

            if (trans == gold_trans)
            {
                state.advance(trans_[trans]);
            }
            else
            {
                auto& update = updates[std::this_thread::get_id()];
                for (const auto& feat : feats)
                {
                    auto& wv = update[feat.first];
                    wv[gold_trans] += feat.second;
                    wv[trans] -= feat.second;
                }
                break;
            }
        }
    });

    // Reduce partial results down to final update vector
    for (const auto& thread_update : updates)
    {
        for (const auto& feat : thread_update.second)
        {
            auto& wv = update[feat.first];
            for (const auto& weight : feat.second)
                wv[weight.first] += weight.second;
        }
    }
    return update;
}

auto sr_parser::best_transition(
    const feature_vector& features) const -> trans_id
{
    weight_vector class_scores;
    for (const auto& feat : features)
    {
        const auto& name = feat.first;
        auto val = feat.second;

        auto it = weights_.find(name);
        if (it == weights_.end())
            continue;

        for (const auto& trans_weight : it->second)
        {
            auto tid = trans_weight.first;
            auto trans_w = trans_weight.second;

            class_scores[tid] += val * trans_w;
        }
    }

    auto best_score = std::numeric_limits<float>::lowest();
    trans_id best_trans{};
    for (const auto& score : class_scores)
    {
        if (score.second > best_score)
        {
            best_trans = score.first;
            best_score = score.second;
        }
    }

    return best_trans;
}

auto sr_parser::featurize(const state& state) const -> feature_vector
{
    feature_vector feats;

    unigram_featurize(state, feats);
    bigram_featurize(state, feats);
    trigram_featurize(state, feats);
    children_featurize(state, feats);

    return feats;
}

void sr_parser::unigram_featurize(const state& state,
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

void sr_parser::unigram_stack_feats(const node* n, std::string prefix,
                                    feature_vector& feats) const
{
    head_info hi{n};

    feats[prefix + "c=" + (std::string)n->category()] = 1;
    feats[prefix + "t=" + hi.head_tag] = 1;
    feats[prefix + "wc=" + hi.head_word + "-" + (std::string)n->category()] = 1;
    feats[prefix + "wt=" + hi.head_word + "-" + hi.head_tag] = 1;
    feats[prefix + "tc=" + hi.head_tag + "-" + (std::string)n->category()] = 1;
}

void sr_parser::bigram_featurize(const state& state,
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

void sr_parser::trigram_featurize(const state& state,
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

void sr_parser::children_featurize(const state& state,
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

void sr_parser::child_feats(const node* n, std::string prefix,
                            feature_vector& feats, bool doubs) const
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

void sr_parser::condense()
{
    // build feature set
    std::vector<std::string> features;
    features.reserve(weights_.size());
    for (const auto& feat_vec : weights_)
        features.push_back(feat_vec.first);

    uint64_t nnz = 0;
    for (const auto& feat : features)
    {
        auto it = weights_.find(feat);
        it->second.condense();
        if (it->second.empty())
            weights_.erase(it);
        else
            nnz += it->second.size();
    }

    LOG(info) << "Number of nonzero weights: " << nnz << ENDLG;
}

void sr_parser::transition_map::save(const std::string& prefix) const
{
    std::ofstream store{prefix + "/parser.trans", std::ios::binary};

    io::write_binary(store, transitions_.size());
    for (const auto& trans : transitions_)
    {
        switch (trans.type())
        {
            case transition::type_t::SHIFT:
                io::write_binary(store, 0);
                break;

            case transition::type_t::REDUCE_L:
                io::write_binary(store, 1);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::REDUCE_R:
                io::write_binary(store, 2);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::UNARY:
                io::write_binary(store, 3);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::FINALIZE:
                io::write_binary(store, 4);
                break;

            case transition::type_t::IDLE:
                io::write_binary(store, 5);
                break;
        }
    }
}

void sr_parser::save(const std::string& prefix) const
{
    trans_.save(prefix);
    std::ofstream model{prefix + "/parser.model", std::ios::binary};

    io::write_binary(model, weights_.size());
    for (const auto& feat_vec : weights_)
    {
        const auto& feat = feat_vec.first;
        const auto& weights = feat_vec.second;

        io::write_binary(model, feat);
        io::write_binary(model, weights.size());

        for (const auto& weight : weights)
        {
            auto tid = weight.first;
            int val = weight.second;
            io::write_binary(model, tid);
            io::write_binary(model, val);
        }
    }
}

sr_parser::transition_map::transition_map(const std::string& prefix)
{
    std::ifstream store{prefix + "/parser.trans", std::ios::binary};

    size_t num_trans;
    io::read_binary(store, num_trans);

    if (!store)
        throw exception{"malformed transitions model file"};

    transitions_.reserve(num_trans);
    for (size_t i = 0; i < num_trans; ++i)
    {
        if (!store)
            throw exception{"malformed transition model file (too few "
                            "transitions written)"};

        int trans_type;
        io::read_binary(store, trans_type);

        util::optional<transition> trans;
        if (trans_type == 0)
        {
            trans = transition{transition::type_t::SHIFT};
        }
        else if (trans_type == 1)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::REDUCE_L, lbl};
        }
        else if (trans_type == 2)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::REDUCE_R, lbl};
        }
        else if (trans_type == 3)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::UNARY, lbl};
        }
        else if (trans_type == 4)
        {
            trans = transition{transition::type_t::FINALIZE};
        }
        else if (trans_type == 5)
        {
            trans = transition{transition::type_t::IDLE};
        }
        else
        {
            throw exception{"invalid transition identifier in model file"};
        }

        auto id = static_cast<trans_id>(map_.size());
        map_[*trans] = id;
        transitions_.emplace_back(std::move(*trans));
    }
}

void sr_parser::load(const std::string& prefix)
{
    std::ifstream model{prefix + "/parser.model", std::ios::binary};

    if (!model)
        throw exception{"model file not found"};

    size_t num_feats;
    io::read_binary(model, num_feats);

    for (size_t i = 0; i < num_feats; ++i)
    {
        if (!model)
            throw exception{"malformed model file (too few features written)"};

        std::string feature_name;
        io::read_binary(model, feature_name);

        size_t num_trans;
        io::read_binary(model, num_trans);

        for (size_t j = 0; j < num_trans; ++j)
        {
            if (!model)
                throw exception{"malformed model file (too few transitions "
                                "written for feature)"};

            trans_id tid;
            float val;
            io::read_binary(model, tid);
            io::read_binary(model, val);

            weights_[feature_name][tid] = val;
        }
    }
}
}
}
