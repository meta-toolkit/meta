/**
 * @file porter2_stemmer.cpp
 * @author Chase Geigle
 */

#include "analyzers/filters/porter2_stemmer.h"
#include "porter2_stemmer.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const std::string porter2_stemmer::id = "porter2-stemmer";

porter2_stemmer::porter2_stemmer(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    next_token();
}

porter2_stemmer::porter2_stemmer(const porter2_stemmer& other)
    : source_{other.source_->clone()}, token_{other.token_}
{
    // nothing
}

void porter2_stemmer::set_content(const std::string& content)
{
    source_->set_content(content);
    next_token();
}

std::string porter2_stemmer::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

void porter2_stemmer::next_token()
{
    while (*source_)
    {
        auto tok = source_->next();
        Porter2Stemmer::stem(tok);
        if (!tok.empty())
        {
            token_ = tok;
            return;
        }
    }
    token_ = util::nullopt;
}

porter2_stemmer::operator bool() const
{
    return static_cast<bool>(token_);
}
}
}
}
