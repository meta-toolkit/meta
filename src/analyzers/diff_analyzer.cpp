/**
 * @file diff_analyzer.cpp
 * @author Sean Massung
 */

#include <string>
#include <vector>

#include "corpus/document.h"
#include "analyzers/diff_analyzer.h"
#include "analyzers/token_stream.h"

namespace meta
{
namespace analyzers
{

const std::string diff_analyzer::id = "diff";

diff_analyzer::diff_analyzer(const cpptoml::toml_group& config,
                             std::unique_ptr<token_stream> stream)
    : stream_{std::move(stream)}, diff_{config}
{
    // nothing
}

diff_analyzer::diff_analyzer(const diff_analyzer& other)
    : stream_{other.stream_->clone()}, diff_{other.diff_}
{
    // nothing
}

void diff_analyzer::tokenize(corpus::document& doc)
{
    // first, get tokens
    stream_->set_content(get_content(doc));
    std::vector<std::string> tokens;
    while (*stream_)
        tokens.push_back(stream_->next());

    doc.increment(tokens[0], 1);
}

template <>
std::unique_ptr<analyzer>
    make_analyzer<diff_analyzer>(const cpptoml::toml_group& global,
                                 const cpptoml::toml_group& config)
{
    auto filts = analyzer::load_filters(global, config);
    auto diff_config = global.get_group("diff-config");
    if (!diff_config)
        throw analyzer::analyzer_exception{
            "diff-config section needed for diff analyzer"};
    return make_unique<diff_analyzer>(*diff_config, std::move(filts));
}
}
}
