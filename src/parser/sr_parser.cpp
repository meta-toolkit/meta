/**
 * @file sr_parser.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <fstream>

#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/parallel/parallel_for.h"
#include "meta/parser/sr_parser.h"
#include "meta/parser/state.h"
#include "meta/parser/state_analyzer.h"
#include "meta/parser/training_data.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/parser/trees/visitors/debinarizer.h"
#include "meta/util/fixed_heap.h"
#include "meta/util/progress.h"
#include "meta/util/range.h"
#include "meta/util/time.h"

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
    if (sentence.size() == 0)
        return {make_unique<internal_node>("ROOT"_cl)};

    state_analyzer analyzer;
    state st{sentence};

    if (beam_size_ == 1)
    {
        while (!st.finalized())
        {
            auto feats = analyzer.featurize(st);
            auto tid = best_transition(feats, st, true);
            auto trans = trans_.at(tid);

            if (!st.legal(trans))
                trans = st.emergency_transition();

            st = st.advance(trans);
        }

        assert(st.stack_size() == 1 && st.queue_size() == 0);

        parse_tree tree{st.stack_item(0)->clone()};
        debinarizer debin;
        tree.transform(debin);

        return tree;
    }
    else
    {
        using scored_state = std::pair<state, float>;

        struct comparator
        {
            bool operator()(const scored_state& lhs,
                            const scored_state& rhs) const
            {
                return std::get<1>(lhs) > std::get<1>(rhs);
            };
        };

        auto fin = [](const scored_state& ss)
        {
            return std::get<0>(ss).finalized();
        };

        using fixed_heap = util::fixed_heap<scored_state, comparator>;

        fixed_heap agenda{beam_size_, comparator{}};
        agenda.emplace(st, 0);

        while (!std::all_of(agenda.begin(), agenda.end(), fin))
        {
            fixed_heap new_agenda{beam_size_, comparator{}};

            for (const auto& ss : agenda)
            {
                const auto& c_state = std::get<0>(ss);
                auto score = std::get<1>(ss);

                auto feats = analyzer.featurize(c_state);

                auto transitions
                    = best_transitions(feats, c_state, beam_size_, true);

                for (const auto& scored_trans : transitions)
                {
                    auto trans = std::get<0>(scored_trans);
                    auto t_score = std::get<1>(scored_trans);

                    new_agenda.emplace(c_state.advance(trans_.at(trans)),
                                       score + t_score);
                }
            }

            if (new_agenda.size() == 0)
            {
                for (const auto& ss : agenda)
                {
                    const auto& c_state = std::get<0>(ss);
                    auto score = std::get<1>(ss);

                    auto trans = c_state.emergency_transition();
                    new_agenda.emplace(c_state.advance(trans), score);
                }
            }

            if (new_agenda.size() == 0)
                throw sr_parser_exception{"unparsable"};

            agenda = std::move(new_agenda);
        }

        // min because comp is backwards
        auto best
            = std::min_element(agenda.begin(), agenda.end(), comparator{});

        parse_tree tree{std::get<0>(*best).stack_item(0)->clone()};
        debinarizer debin;
        tree.transform(debin);

        return tree;
    }
}

void sr_parser::train(std::vector<parse_tree>& trees, training_options options)
{
    if (options.algorithm == training_algorithm::BEAM_SEARCH)
        beam_size_ = options.beam_size;

    training_data data{trees, options.seed};
    trans_ = data.preprocess();

    LOG(info) << "Found " << trans_.size() << " transitions" << ENDLG;

    parallel::thread_pool pool{options.num_threads};

    classify::linear_model<std::string, float, trans_id> for_avg;
    uint64_t total_updates = 0;
    for (uint64_t iter = 1; iter <= options.max_iterations; ++iter)
    {
        uint64_t num_correct = 0;
        uint64_t num_incorrect = 0;

        auto time = common::time(
            [&]()
            {
                printing::progress progress{" > Iteration "
                                                + std::to_string(iter) + ": ",
                                            trees.size()};
                data.shuffle();

                for (size_t start = 0; start < data.size();
                     start += options.batch_size)
                {
                    progress(start);
                    auto end = std::min<uint64_t>(start + options.batch_size,
                                                  data.size());

                    auto result
                        = train_batch({data, start, end}, pool, options);

                    ++total_updates;
                    model_.update(std::get<0>(result));
                    for_avg.update(std::get<0>(result), total_updates - 1);

                    num_correct += std::get<1>(result);
                    num_incorrect += std::get<2>(result);
                }
            });

        LOG(info) << "Took " << time.count() / 1000.0 << "s" << ENDLG;

        LOG(info) << "Correct transitions: " << num_correct
                  << ", incorrect transitions: " << num_incorrect << ENDLG;

        model_.condense(true);
        for_avg.condense(false);
    }

    // update weights to be average over all parameters
    model_.update(for_avg.weights(), -1.0f / total_updates);
}

auto sr_parser::train_batch(training_batch batch, parallel::thread_pool& pool,
                            const training_options& options)
    -> std::tuple<weight_vectors, uint64_t, uint64_t>
{
    // TODO: real beam search
    std::tuple<weight_vectors, uint64_t, uint64_t> result;

    auto range = util::range(batch.start, batch.end - 1); // inclusive range

    // Perform a reduction across threads: each thread stores its update in
    // a separate location, and we then add them all up after we join
    util::sparse_vector<std::thread::id, weight_vectors> updates;
    for (const auto& tid : pool.thread_ids())
        updates[tid] = {};

    std::atomic<uint64_t> num_correct{0};
    std::atomic<uint64_t> num_incorrect{0};
    parallel::parallel_for(
        range.begin(), range.end(), pool, [&](size_t i)
        {
            auto& tree = batch.data.tree(i);
            auto& transitions = batch.data.transitions(i);
            auto& update = updates[std::this_thread::get_id()];

            auto res = train_instance(tree, transitions, options, update);
            num_correct += res.first;
            num_incorrect += res.second;
        });

    // Reduce partial results down to final update vector
    for (const auto& thread_update : updates)
    {
        for (const auto& feat : thread_update.second)
        {
            auto& wv = std::get<0>(result)[feat.first];
            for (const auto& weight : feat.second)
                wv[weight.first] += weight.second;
        }
    }
    std::get<1>(result) = num_correct.load();
    std::get<2>(result) = num_incorrect.load();
    return result;
}

std::pair<uint64_t, uint64_t> sr_parser::train_instance(
    const parse_tree& tree, const std::vector<trans_id>& transitions,
    const training_options& options, weight_vectors& update) const
{
    switch (options.algorithm)
    {
        case training_algorithm::EARLY_TERMINATION:
            return train_early_termination(tree, transitions, update);

        case training_algorithm::BEAM_SEARCH:
            return train_beam_search(tree, transitions, options, update);

        default:
            throw sr_parser_exception{"Not yet implemented"};
    }
}

std::pair<uint64_t, uint64_t>
sr_parser::train_early_termination(const parse_tree& tree,
                                   const std::vector<trans_id>& transitions,
                                   weight_vectors& update) const
{
    std::pair<uint64_t, uint64_t> result{0, 0};
    state state{tree};
    state_analyzer analyzer;

    for (const auto& gold_trans : transitions)
    {
        auto feats = analyzer.featurize(state);
        auto trans = best_transition(feats, state);

        if (trans == gold_trans)
        {
            state = state.advance(trans_.at(trans));
            ++result.first;
        }
        else
        {
            for (const auto& feat : feats)
            {
                auto& wv = update[feat.first];
                wv[gold_trans] += feat.second;
                wv[trans] -= feat.second;
            }
            ++result.second;
            break;
        }
    }
    return result;
}

std::pair<uint64_t, uint64_t> sr_parser::train_beam_search(
    const parse_tree& tree, const std::vector<trans_id>& transitions,
    const training_options& options, weight_vectors& update) const
{
    std::pair<uint64_t, uint64_t> result{0, 0};
    state gold_state{tree};
    state_analyzer analyzer;

    using scored_state = std::tuple<state, double, bool>;
    // get<0>() is the state
    // get<1>() is the score
    // get<2>() is whether or not it is the same as the gold state

    struct score_compare
    {
        bool operator()(const scored_state& a, const scored_state& b) const
        {
            return std::get<1>(a) > std::get<1>(b);
        }
    };

    using fixed_heap = util::fixed_heap<scored_state, score_compare>;

    fixed_heap agenda{options.beam_size, score_compare{}};
    agenda.emplace(state{tree}, 0, true);

    for (const auto& gold_trans : transitions)
    {
        fixed_heap new_agenda{options.beam_size, score_compare{}};

        // keep track if any of the new states is the gold one
        bool any_gold = false;

        // keep track of the best state that is currently on the agenda
        util::optional<scored_state> best_state;

        // keep track of the best state that is on the new agenda
        util::optional<scored_state> best_new_state;

        // keep track of the transition taken for moving from best_state to
        // best_new_state
        trans_id best_trans;

        for (const auto& ss : agenda)
        {
            const auto& st = std::get<0>(ss);
            const auto& score = std::get<1>(ss);
            bool is_gold = std::get<2>(ss);

            auto feats = analyzer.featurize(std::get<0>(ss));

            auto transitions
                = best_transitions(feats, st, options.beam_size, true);

            for (const auto& scored_trans : transitions)
            {
                auto trans = std::get<0>(scored_trans);
                auto t_score = std::get<1>(scored_trans);

                auto new_state = st.advance(trans_.at(trans));
                auto new_score = score + t_score;
                auto new_is_gold = is_gold && trans == gold_trans;

                any_gold = any_gold || new_is_gold;

                auto new_ss
                    = std::make_tuple(new_state, new_score, new_is_gold);

                if (!best_new_state
                    || std::get<1>(new_ss) > std::get<1>(*best_new_state))
                {
                    best_state = ss;
                    best_new_state = new_ss;
                    best_trans = trans;
                }

                new_agenda.emplace(new_state, new_score, new_is_gold);
            }
        }

        assert(new_agenda.size() <= options.beam_size);

        if (!best_new_state || !std::get<2>(*best_new_state))
        {
            ++result.second;

            if (best_state)
            {
                auto best_feats = analyzer.featurize(std::get<0>(*best_state));

                for (const auto& feat : best_feats)
                    update[feat.first][best_trans] -= feat.second;
            }

            {
                auto gold_feats = analyzer.featurize(gold_state);
                for (const auto& feat : gold_feats)
                    update[feat.first][gold_trans] += feat.second;
            }
        }
        else
        {
            ++result.first;
        }

        if (!any_gold)
            break;

        gold_state = gold_state.advance(trans_.at(gold_trans));
        agenda = std::move(new_agenda);
    }

    return result;
}

auto sr_parser::best_transition(const feature_vector& features,
                                const state& state,
                                bool check_legality /* = false */) const
    -> trans_id
{
    return model_.best_class(features, [&](trans_id tid)
                             {
                                 return !check_legality
                                        || state.legal(trans_.at(tid));
                             });
}

auto sr_parser::best_transitions(const feature_vector& features,
                                 const state& state, size_t num,
                                 bool check_legality) const
    -> std::vector<scored_trans>
{
    return model_.best_classes(features, num, [&](trans_id tid)
                               {
                                   return !check_legality
                                          || state.legal(trans_.at(tid));
                               });
}

void sr_parser::save(const std::string& prefix) const
{
    trans_.save(prefix);
    io::gzofstream model{prefix + "/parser.model.gz"};
    io::packed::write(model, beam_size_);
    model_.save(model);
}

void sr_parser::load(const std::string& prefix)
{
    auto model_file = prefix + "/parser.model.gz";
    if (!filesystem::file_exists(model_file))
        throw sr_parser_exception{"model file not found: " + model_file};

    io::gzifstream model{model_file};
    io::packed::read(model, beam_size_);
    model_.load(model);
}
}
}
