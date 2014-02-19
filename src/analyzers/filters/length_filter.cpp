/**
 * @file length_filter.cpp
 * @author Chase Geigle
 */

#include "analyzers/filters/length_filter.h"

namespace meta
{
namespace analyzers
{

length_filter::length_filter(token_stream& source, uint64_t min, uint64_t max)
    : source_(source), min_length_{min}, max_length_{max}
{
    next_token();
}

std::string length_filter::next()
{
    auto tok = *token_;
    next_token();
    return tok;
}

length_filter::operator bool() const
{
    return token_ || source_;
}

void length_filter::next_token()
{
    if (!source_)
    {
        token_ = util::nullopt;
        return;
    }

    while (source_)
    {
        auto tok = source_.next();
        if (tok.length() >= min_length_ && tok.length() <= max_length_)
        {
            token_ = tok;
            break;
        }
    }
}

}
}
