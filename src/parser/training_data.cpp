/**
 * @file training_data.cpp
 * @author Chase Geigle
 */

#include "meta/parser/training_data.h"
#include "meta/parser/trees/visitors/annotation_remover.h"
#include "meta/parser/trees/visitors/empty_remover.h"
#include "meta/parser/trees/visitors/unary_chain_remover.h"
#include "meta/parser/trees/visitors/multi_transformer.h"
#include "meta/parser/trees/visitors/head_finder.h"
#include "meta/parser/trees/visitors/binarizer.h"
#include "meta/parser/trees/visitors/debinarizer.h"
#include "meta/parser/transition_finder.h"
#include "meta/parser/trees/visitors/leaf_node_finder.h"
#include "meta/util/progress.h"

namespace meta
{
namespace parser
{

sr_parser::training_data::training_data(
    std::vector<parse_tree>& trs, std::default_random_engine::result_type seed)
    : trees_(trs), indices_(trs.size()), rng_{seed}
{
    std::iota(indices_.begin(), indices_.end(), 0ul);
}

auto sr_parser::training_data::preprocess() -> transition_map
{
    transition_map trans_map;

    multi_transformer<annotation_remover, empty_remover, unary_chain_remover>
        transformer;

    head_finder hf;
    binarizer bin;

    printing::progress progress{" > Preprocessing training trees: ",
                                trees_.size()};
    size_t idx = 0;
    for (auto& tree : trees_)
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
        all_transitions_.emplace_back(std::move(tids));
    }

    return trans_map;
}

void sr_parser::training_data::shuffle()
{
    std::shuffle(indices_.begin(), indices_.end(), rng_);
}

size_t sr_parser::training_data::size() const
{
    return indices_.size();
}

const parse_tree& sr_parser::training_data::tree(size_t idx) const
{
    return trees_[indices_[idx]];
}

auto sr_parser::training_data::transitions(
    size_t idx) const -> const std::vector<trans_id> &
{
    return all_transitions_[indices_[idx]];
}
}
}
