/**
 * @file alpha_filter.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include "analyzers/filters/alpha_filter.h"
#include "util/utf.h"

namespace meta
{
namespace analyzers
{

alpha_filter::alpha_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

alpha_filter::alpha_filter(const alpha_filter& other)
    : source_{other.source_->clone()}
{
    // nothing
}

void alpha_filter::set_content(const std::string& content)
{
    source_->set_content(content);
}

std::string alpha_filter::next()
{
    auto tok = source_->next();
    if (tok == "<s>" || tok == "</s>")
        return tok;

    return utf::remove_if(tok, [](uint32_t codepoint)
    {
        return !utf::isalpha(codepoint) && codepoint != '\'';
    });
}

alpha_filter::operator bool() const
{
    return *source_;
}


}
}
