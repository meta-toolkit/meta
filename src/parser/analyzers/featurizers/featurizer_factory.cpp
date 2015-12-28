/**
 * @file featurizer_factory.cpp
 * @author Chase Geigle
 */

#include "meta/parser/analyzers/featurizers/featurizer_factory.h"
#include "meta/parser/analyzers/featurizers/all.h"

namespace meta
{
namespace analyzers
{

template <class T>
template <class Featurizer>
void featurizer_factory<T>::register_featurizer()
{
    // this-> needed to find  the add() method in dependent base class
    this->add(Featurizer::id, make_featurizer<Featurizer>);
}

template <class T>
featurizer_factory<T>::featurizer_factory()
{
    // built-in featurizer
    register_featurizer<branch_featurizer<T>>();
    register_featurizer<depth_featurizer<T>>();
    register_featurizer<semi_skeleton_featurizer<T>>();
    register_featurizer<skeleton_featurizer<T>>();
    register_featurizer<subtree_featurizer<T>>();
    register_featurizer<tag_featurizer<T>>();
}

template class featurizer_factory<uint64_t>;
template class featurizer_factory<double>;
}
}
