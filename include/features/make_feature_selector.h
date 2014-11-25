/**
 * @file make_feature_selector.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MAKE_FEATURE_SELECTOR_H_
#define META_MAKE_FEATURE_SELECTOR_H_

#include <string>

#include "cpptoml.h"
#include "features/feature_selector.h"

namespace meta
{
namespace features
{
/**
 * Factory method for creating feature selection algorithms.
 * @param config_file The path to the configuration file to create selector
 * @param fwd_idx The forward_index to perform feature selection on
 * @param args any additional arguments to forward to the constructor (usually
 * none)
 * @return a properly initialized feature_selector
 */
template <class Selector, class ForwardIndex, class... Args>
std::shared_ptr<Selector> make_selector(const std::string& config_file,
                                        ForwardIndex fwd_idx, Args&&... args)
{
    auto config = cpptoml::parse_file(config_file);
    auto group = config.get_group("features");
    if (!group)
        throw std::runtime_error{"[features] group missing from config file"};

    auto prefix = group->get_as<std::string>("prefix");
    if (!prefix)
        throw std::runtime_error{"no prefix in [features] group"};

    // can't use make_shared since Selector constructors are private
    auto selector = std::shared_ptr<Selector>{
        new Selector(*prefix, std::move(fwd_idx), std::forward<Args>(args)...)};

    uint64_t features_per_class = 20;
    auto num_features = group->get_as<int64_t>("features-per-class");
    if (num_features)
        features_per_class = *num_features;

    selector->init(features_per_class);
    return selector;
}
}
}

#endif
