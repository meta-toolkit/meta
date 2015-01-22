/**
 * @file sr_parser.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <fstream>

#include "io/binary.h"
#include "logging/logger.h"
#include "parallel/parallel_for.h"
#include "parser/sr_parser.h"
#include "parser/state.h"
#include "parser/state_analyzer.h"
#include "parser/training_data.h"
#include "parser/trees/internal_node.h"
#include "parser/trees/leaf_node.h"
#include "parser/trees/visitors/debinarizer.h"
#include "util/progress.h"
#include "util/range.h"

namespace meta
{
namespace parser
{

sr_parser::sr_parser(const std::string& prefix) : trans_{prefix}
{
    load(prefix);
}

parse_tree sr_parser::parse(const sequence::sequence& sentence) const
{
    state state{sentence};
    state_analyzer analyzer;

    while (!state.finalized())
    {
        auto feats = analyzer.featurize(state);
        auto tid = best_transition(feats, state);

        state.advance(trans_.at(tid));
    }

    assert(state.stack_size() == 1 && state.queue_size() == 0);

    parse_tree tree{state.stack_item(0)->clone()};
    debinarizer debin;
    tree.transform(debin);

    return tree;
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

            auto update = train_batch({data, start, end}, pool, options);

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

auto sr_parser::train_batch(training_batch batch, parallel::thread_pool& pool,
                            const training_options& options) -> weight_vectors
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
        auto& update = updates[std::this_thread::get_id()];

        train_instance(tree, transitions, options, update);
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

void sr_parser::train_instance(const parse_tree& tree,
                               const std::vector<trans_id>& transitions,
                               const training_options& options,
                               weight_vectors& update) const
{
    // TODO: add beam search
    switch (options.algorithm)
    {
        case training_algorithm::EARLY_TERMINATION:
            train_early_termination(tree, transitions, update);
            break;
    }
}

void
    sr_parser::train_early_termination(const parse_tree& tree,
                                       const std::vector<trans_id>& transitions,
                                       weight_vectors& update) const
{
    state state{tree};
    state_analyzer analyzer;

    for (const auto& gold_trans : transitions)
    {
        auto feats = analyzer.featurize(state);
        auto trans = best_transition(feats, state);

        if (trans == gold_trans)
        {
            state.advance(trans_.at(trans));
        }
        else
        {
            for (const auto& feat : feats)
            {
                auto& wv = update[feat.first];
                wv[gold_trans] += feat.second;
                wv[trans] -= feat.second;
            }
            break;
        }
    }
}

auto sr_parser::best_transition(
    const feature_vector& features, const state& state,
    bool check_legality /* = false */) const -> trans_id
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
        auto tid = score.first;
        const auto& trans = trans_.at(tid);
        if (score.second > best_score
            && (!check_legality || state.legal(trans)))
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

    LOG(info) << "Number of total features: " << weights_.size() << ENDLG;
    LOG(info) << "Number of nonzero weights: " << nnz << ENDLG;
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
