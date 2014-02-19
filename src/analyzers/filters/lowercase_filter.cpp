/**
 * @file lowercase_filter.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cctype>
#include "analyzers/filters/lowercase_filter.h"

namespace meta
{
namespace analyzers
{

lowercase_filter::lowercase_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

void lowercase_filter::set_content(const std::string& content)
{
    source_->set_content(content);
}

std::string lowercase_filter::next()
{
    auto tok = source_->next();
    std::transform(tok.begin(), tok.end(), tok.begin(), ::tolower);
    return tok;
}

lowercase_filter::operator bool() const
{
    return *source_;
}

}
}
