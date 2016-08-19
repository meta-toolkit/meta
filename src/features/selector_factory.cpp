/**
 * @file selector_factory.cpp
 * @author Sean Massung
 */

#include "meta/features/selector_factory.h"
#include "cpptoml.h"
#include "meta/features/all.h"

namespace meta
{
namespace features
{

template <class Selector>
void selector_factory::reg()
{
    add(Selector::id, factory_make_selector<Selector>);
}

selector_factory::selector_factory()
{
    // built-in feature-selection algorithms
    reg<information_gain>();
    reg<chi_square>();
    reg<correlation_coefficient>();
    reg<odds_ratio>();
}
}
}
