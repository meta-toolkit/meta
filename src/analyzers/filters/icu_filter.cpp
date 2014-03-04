/**
 * @file icu_filter.cpp
 * @author Chase Geigle
 */

#include "analyzers/filters/icu_filter.h"
#include "cpptoml.h"

namespace meta
{
namespace analyzers
{

icu_filter::icu_filter(std::unique_ptr<token_stream> source,
                       const std::string& id)
    : source_{std::move(source)}, trans_{id}
{
    // nothing
}

icu_filter::icu_filter(const icu_filter& other)
    : source_{other.source_->clone()}, trans_{other.trans_}
{
    // nothing
}

void icu_filter::set_content(const std::string& content)
{
    source_->set_content(content);
}

std::string icu_filter::next()
{
    auto tok = source_->next();
    if (tok == "<s>" || tok == "</s>")
        return tok;
    return trans_(tok);
}

icu_filter::operator bool() const
{
    return *source_;
}

template <>
std::unique_ptr<token_stream>
    make_filter<icu_filter>(std::unique_ptr<token_stream> src,
                            const cpptoml::toml_group& config)
{
    if (auto id = config.get_as<std::string>("id"))
        return make_unique<icu_filter>(std::move(src), *id);
    throw token_stream::token_stream_exception{
        "icu_filter requires id to be specified in config"};
}

}
}
