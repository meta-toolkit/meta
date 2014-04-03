/**
 * @file alpha_filter.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include "analyzers/filters/alpha_filter.h"
#include "utf/utf.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const std::string alpha_filter::id = "alpha";

alpha_filter::alpha_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    next_token();
}

alpha_filter::alpha_filter(const alpha_filter& other)
    : source_{other.source_->clone()}, token_{other.token_}
{
    // nothing
}

void alpha_filter::set_content(const std::string& content)
{
    source_->set_content(content);
    next_token();
}

std::string alpha_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

void alpha_filter::next_token()
{
    while (*source_)
    {
        auto tok = source_->next();
        if (tok == "<s>" || tok == "</s>")
        {
            token_ = tok;
            return;
        }

        auto filt = utf::remove_if(tok, [](uint32_t codepoint)
        { return !utf::isalpha(codepoint) && codepoint != '\''; });
        if (!filt.empty())
        {
            token_ = filt;
            return;
        }
    }
    token_ = util::nullopt;
}

alpha_filter::operator bool() const
{
    return static_cast<bool>(token_);
}
}
}
}
