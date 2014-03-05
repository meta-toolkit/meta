/**
 * @file analyzer_factory.cpp
 * @author Chase Geigle
 */

#include "analyzers/analyzer_factory.h"
#include "analyzers/all.h"

namespace meta
{
namespace analyzers
{

template <class Analyzer>
void analyzer_factory::register_analyzer()
{
    add(Analyzer::id, make_analyzer<Analyzer>);
}

analyzer_factory::analyzer_factory()
{
    // built-in analyzers
    register_analyzer<branch_analyzer>();
    register_analyzer<depth_analyzer>();
    register_analyzer<semi_skeleton_analyzer>();
    register_analyzer<skeleton_analyzer>();
    register_analyzer<subtree_analyzer>();
    register_analyzer<tag_analyzer>();
    register_analyzer<ngram_word_analyzer>();
    register_analyzer<ngram_lex_analyzer>();
    register_analyzer<ngram_pos_analyzer>();
    register_analyzer<libsvm_analyzer>();
}

auto analyzer_factory::create(const std::string& identifier,
                              const cpptoml::toml_group& global,
                              const cpptoml::toml_group& config) -> pointer
{
    if (methods_.find(identifier) == methods_.end())
        throw exception{"unrecognized analyzer id"};
    return methods_[identifier](global, config);
}

}
}
