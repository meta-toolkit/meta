/**
 * @file sr_parser.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <fstream>
#include <numeric>

#include "io/binary.h"
#include "parallel/parallel_for.h"
#include "parser/sr_parser.h"
#include "parser/trees/visitors/annotation_remover.h"
#include "parser/trees/visitors/empty_remover.h"
#include "parser/trees/visitors/unary_chain_remover.h"
#include "parser/trees/visitors/multi_transformer.h"
#include "parser/trees/visitors/head_finder.h"
#include "parser/trees/visitors/binarizer.h"
#include "parser/trees/visitors/debinarizer.h"
#include "parser/trees/visitors/transition_finder.h"
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

class leaf_node_finder : public const_visitor<void>
{
  public:
    void operator()(const leaf_node& ln) override
    {
        leaves_.push_back(make_unique<leaf_node>(ln));
    }

    void operator()(const internal_node& in) override
    {
        in.each_child([&](const node* n)
                      {
                          n->accept(*this);
                      });
    }

    std::vector<std::unique_ptr<leaf_node>> leaves()
    {
        return std::move(leaves_);
    }

  private:
    std::vector<std::unique_ptr<leaf_node>> leaves_;
};

sr_parser::parser_state::parser_state(const parse_tree& tree)
{
    leaf_node_finder lnf;
    tree.visit(lnf);
    queue = lnf.leaves();
    q_idx = 0;
    done = false;
}

void sr_parser::parser_state::advance(transition trans)
{
    // TODO: Inheritance hierarchy?
    switch (trans.type())
    {
        case transition::type_t::SHIFT:
        {
            stack = stack.push(queue[q_idx]->clone());
            ++q_idx;
        }
        break;

        case transition::type_t::REDUCE_L:
        case transition::type_t::REDUCE_R:
        {
            auto right = stack.peek()->clone();
            stack = stack.pop();
            auto left = stack.peek()->clone();
            stack = stack.pop();

            auto bin = make_unique<internal_node>(trans.label());
            bin->add_child(std::move(left));
            bin->add_child(std::move(right));

            if (trans.type() == transition::type_t::REDUCE_L)
            {
                bin->head(bin->child(0));
            }
            else
            {
                bin->head(bin->child(1));
            }

            stack = stack.push(std::move(bin));
        }
        break;

        case transition::type_t::UNARY:
        {
            auto child = stack.peek()->clone();
            stack = stack.pop();

            auto un = make_unique<internal_node>(trans.label());
            un->add_child(std::move(child));
            un->head(un->child(0));

            stack = stack.push(std::move(un));
        }
        break;

        case transition::type_t::FINALIZE:
        {
            done = true;
        }
        break;

        case transition::type_t::IDLE:
        {
            // nothing
        }
        break;
    }
}

sr_parser::training_data::training_data(training_options opt,
                                        std::vector<parse_tree>& trs)
    : options(opt), trees(trs), indices(trs.size()), rng{opt.seed}
{
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

        all_transitions.emplace_back(std::move(trans.transitions()));
    }
    std::iota(indices.begin(), indices.end(), 0ul);
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

const std::vector<transition>&
    sr_parser::training_data::transitions(size_t idx) const
{
    return all_transitions[indices[idx]];
}

sr_parser::sr_parser(const std::string& prefix)
{
    load(prefix);
}

void sr_parser::train(std::vector<parse_tree>& trees, training_options options)
{
    training_data data{options, trees};

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

        parser_state state{tree};

        for (const auto& gold_trans : transitions)
        {
            auto feats = featurize(state);
            auto trans = best_transition(feats);

            if (trans == gold_trans)
            {
                state.advance(trans);
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

transition sr_parser::best_transition(const feature_vector& features) const
{
    weight_vector class_scores;
    for (const auto& feat : features)
    {
        const auto& name = feat.first;
        double val = feat.second;

        auto it = weights_.find(name);
        if (it == weights_.end())
            continue;

        for (const auto& trans_weight : it->second)
        {
            const auto& trans = trans_weight.first;
            double trans_w = trans_weight.second;

            class_scores[trans] += val * trans_w;
        }
    }

    auto best_score = std::numeric_limits<double>::lowest();
    transition best_trans{transition::type_t::SHIFT};
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

auto sr_parser::featurize(const parser_state& state) const -> feature_vector
{
    feature_vector feats;

    unigram_featurize(state, feats);
    bigram_featurize(state, feats);
    trigram_featurize(state, feats);
    children_featurize(state, feats);

    return feats;
}

void sr_parser::unigram_featurize(const parser_state& state,
                                  feature_vector& feats) const
{
    if (state.stack.size() > 0)
    {
        const auto& s0 = state.stack.peek().get();
        unigram_stack_feats(s0, "s0", feats);
    }

    if (state.stack.size() > 1)
    {
        const auto& s1 = state.stack.pop().peek().get();
        unigram_stack_feats(s1, "s1", feats);
    }

    if (state.stack.size() > 2)
    {
        const auto& s2 = state.stack.pop().pop().peek().get();
        unigram_stack_feats(s2, "s2", feats);
    }

    if (state.stack.size() > 3)
    {
        const auto& s3 = state.stack.pop().pop().pop().peek().get();
        unigram_stack_feats(s3, "s3", feats);
    }

    for (int i = 0; i <= 3 && state.q_idx + i < state.queue.size(); ++i)
    {
        auto word = *state.queue[state.q_idx + i]->word();
        const std::string& tag = state.queue[state.q_idx + i]->category();
        feats["q" + std::to_string(i) + "wt=" + word + "-" + tag] = 1;
    }
}

void sr_parser::unigram_stack_feats(const node* n, std::string prefix,
                                    feature_vector& feats) const
{
    head_info hi{n};

    feats[prefix + "tc=" + hi.head_tag + "-" + (std::string)n->category()] = 1;
    feats[prefix + "wc=" + hi.head_word + "-" + (std::string)n->category()] = 1;
}

void sr_parser::bigram_featurize(const parser_state& state,
                                 feature_vector& feats) const
{
    if (state.stack.size() > 0 && state.q_idx < state.queue.size())
    {
        // can access s0 and q0
        const auto& s0 = state.stack.peek().get();
        const auto& q0 = state.queue[state.q_idx].get();

        head_info s0h{s0};
        head_info q0h{q0};

        feats["s0wq0w=" + s0h.head_word + "-" + q0h.head_word] = 1;
        feats["s0wq0t=" + s0h.head_word + "-" + q0h.head_tag] = 1;
        feats["s0cq0w=" + (std::string)s0->category() + "-" + q0h.head_word]
            = 1;
        feats["s0cq0t=" + (std::string)s0->category() + "-" + q0h.head_tag] = 1;

        if (state.stack.size() > 1)
        {
            // can access s1
            const auto& s1 = state.stack.pop().peek().get();

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

        if (state.q_idx + 1 < state.queue.size())
        {
            // can access q1
            const auto& q1 = state.queue[state.q_idx + 1].get();

            head_info q1h{q1};

            feats["q0wq1w=" + q0h.head_word + "-" + q1h.head_word] = 1;
            feats["q0wq1t=" + q0h.head_word + "-" + q1h.head_tag] = 1;
            feats["q0tq1w=" + q0h.head_tag + "-" + q1h.head_word] = 1;
            feats["q0tq1t=" + q0h.head_tag + "-" + q1h.head_tag] = 1;
        }
    }
}

void sr_parser::trigram_featurize(const parser_state& state,
                                  feature_vector& feats) const
{
    if (state.stack.size() < 2)
        return;

    const auto& s0 = state.stack.peek().get();
    const auto& s1 = state.stack.pop().peek().get();

    head_info s0h{s0};
    head_info s1h{s1};

    if (state.stack.size() > 2)
    {
        // can access s2
        const auto& s2 = state.stack.pop().pop().peek().get();

        head_info s2h{s2};

        feats["s0cs1cs2c=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + (std::string)s2->category()]
            = 1;

        feats["s0ws1cs2c=" + s0h.head_word + "-" + (std::string)s1->category()
              + "-" + (std::string)s2->category()] = 1;

        feats["s0cs1cs2w=" + (std::string)s0->category() + "-"
              + (std::string)s1->category() + "-" + s2h.head_word] = 1;
    }

    if (state.q_idx < state.queue.size())
    {
        // can access q0
        const auto& q0 = state.queue[state.q_idx].get();

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

void sr_parser::children_featurize(const parser_state& state,
                                   feature_vector& feats) const
{
    if (state.stack.size() > 0)
    {
        const auto& s0 = state.stack.peek().get();
        child_feats(s0, "s0", feats, true);
    }

    if (state.stack.size() > 1)
    {
        const auto& s1 = state.stack.peek().get();
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

    for (const auto& feat : features)
    {
        auto it = weights_.find(feat);
        it->second.condense();
        if (it->second.empty())
            weights_.erase(it);
    }
}

void sr_parser::save(const std::string& prefix) const
{
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
            const auto& trans = weight.first;
            double val = weight.second;
            switch (trans.type())
            {
                case transition::type_t::SHIFT:
                    io::write_binary(model, 0);
                    break;

                case transition::type_t::REDUCE_L:
                    io::write_binary(model, 1);
                    io::write_binary(model, trans.label());
                    break;

                case transition::type_t::REDUCE_R:
                    io::write_binary(model, 2);
                    io::write_binary(model, trans.label());
                    break;

                case transition::type_t::UNARY:
                    io::write_binary(model, 3);
                    io::write_binary(model, trans.label());
                    break;

                case transition::type_t::FINALIZE:
                    io::write_binary(model, 4);
                    break;

                case transition::type_t::IDLE:
                    io::write_binary(model, 5);
                    break;
            }
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

            int trans_type;
            double val;
            io::read_binary(model, trans_type);
            io::read_binary(model, val);

            util::optional<transition> trans;
            if (trans_type == 0)
            {
                trans = transition{transition::type_t::SHIFT};
            }
            else if (trans_type == 1)
            {
                class_label lbl;
                io::read_binary(model, lbl);
                trans = transition{transition::type_t::REDUCE_L, lbl};
            }
            else if (trans_type == 2)
            {
                class_label lbl;
                io::read_binary(model, lbl);
                trans = transition{transition::type_t::REDUCE_R, lbl};
            }
            else if (trans_type == 3)
            {
                class_label lbl;
                io::read_binary(model, lbl);
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

            weights_[feature_name][*trans] = val;
        }
    }
}
}
}
