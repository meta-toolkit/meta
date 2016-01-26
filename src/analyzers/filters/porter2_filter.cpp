/**
 * @file porter2_filter.cpp
 * @author Chase Geigle
 */

#include "meta/analyzers/filters/porter2_filter.h"
#include "meta/analyzers/filters/porter2_stemmer.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view porter2_filter::id = "porter2-filter";

porter2_filter::porter2_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    next_token();
}

porter2_filter::porter2_filter(const porter2_filter& other)
    : source_{other.source_->clone()}, token_{other.token_}
{
    // nothing
}

void porter2_filter::set_content(std::string&& content)
{
    source_->set_content(std::move(content));
    next_token();
}

std::string porter2_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

void porter2_filter::next_token()
{
    while (*source_)
    {
        auto tok = source_->next();
        porter2::stem(tok);
        if (!tok.empty())
        {
            token_ = std::move(tok);
            return;
        }
    }
    token_ = util::nullopt;
}

porter2_filter::operator bool() const
{
    return static_cast<bool>(token_);
}
}
}
}
