/**
 * @file training_data.cpp
 * @author Chase Geigle
 */

#include "parser/training_data.h"
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
}
}
