/**
 * @file featurizer_factory.cpp
 * @author Chase Geigle
 */

#include "analyzers/tree/featurizers/featurizer_factory.h"
#include "analyzers/tree/featurizers/all.h"

namespace meta
{
namespace analyzers
{

template <class Featurizer>
void featurizer_factory::register_featurizer()
{
    add(Featurizer::id, make_featurizer<Featurizer>);
}

featurizer_factory::featurizer_factory()
{
    // built-in featurizer
    register_featurizer<branch_featurizer>();
    register_featurizer<depth_featurizer>();
    register_featurizer<semi_skeleton_featurizer>();
    register_featurizer<skeleton_featurizer>();
    register_featurizer<subtree_featurizer>();
    register_featurizer<tag_featurizer>();
}
}
}
