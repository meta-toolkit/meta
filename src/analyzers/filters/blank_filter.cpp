/**
 * @file blank_filter.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "analyzers/filters/blank_filter.h"
#include "utf/utf.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const std::string blank_filter::id = "blank";

blank_filter::blank_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    next_token();
}

blank_filter::blank_filter(const blank_filter& other)
    : source_{other.source_->clone()}, token_{other.token_}
{
    // nothing
}

void blank_filter::set_content(const std::string& content)
{
    token_ = util::nullopt;
    source_->set_content(content);
    next_token();
}

std::string blank_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

blank_filter::operator bool() const
{
    return token_ || *source_;
}

void blank_filter::next_token()
{
    if (!*source_)
    {
        token_ = util::nullopt;
        return;
    }

    while (*source_)
    {
        auto tok = source_->next();
        tok = utf::remove_if(tok, [](uint32_t codepoint)
                             {
            return utf::isblank(codepoint);
        });
        auto len = utf::length(tok);
        if (len > 0)
        {
            token_ = tok;
            return;
        }
    }
    token_ = util::nullopt;
}
}
}
}
