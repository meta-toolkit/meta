/**
 * @file icu_filter.cpp
 * @author Chase Geigle
 */

#include "meta/analyzers/filters/icu_filter.h"
#include "cpptoml.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view icu_filter::id = "icu";

icu_filter::icu_filter(std::unique_ptr<token_stream> source,
                       const std::string& id)
    : source_{std::move(source)}, trans_{id}
{
    next_token();
}

icu_filter::icu_filter(const icu_filter& other)
    : source_{other.source_->clone()},
      trans_{other.trans_},
      token_{other.token_}
{
    // nothing
}

void icu_filter::set_content(std::string&& content)
{
    source_->set_content(std::move(content));
    next_token();
}

std::string icu_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

void icu_filter::next_token()
{
    while (*source_)
    {
        auto tok = source_->next();
        if (tok == "<s>" || tok == "</s>")
        {
            token_ = tok;
            return;
        }
        auto trans = trans_(tok);
        if (!trans.empty())
        {
            token_ = trans;
            return;
        }
    }
    token_ = util::nullopt;
}

icu_filter::operator bool() const
{
    return static_cast<bool>(token_);
}

template <>
std::unique_ptr<token_stream>
    make_filter<icu_filter>(std::unique_ptr<token_stream> src,
                            const cpptoml::table& config)
{
    if (auto id = config.get_as<std::string>("id"))
        return make_unique<icu_filter>(std::move(src), *id);
    throw token_stream_exception{
        "icu_filter requires id to be specified in config"};
}
}
}
}
