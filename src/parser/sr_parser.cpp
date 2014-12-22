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
#include "parser/state_analyzer.h"
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

    state_analyzer analyzer;
    parallel::parallel_for(range.begin(), range.end(), pool, [&](size_t i)
                           {
        auto& tree = batch.data.tree(i);
        auto& transitions = batch.data.transitions(i);

        state state{tree};

        for (const auto& gold_trans : transitions)
        {
            auto feats = analyzer.featurize(state);
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
