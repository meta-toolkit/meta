/**
 * @file lowercase_filter.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cctype>
#include "meta/analyzers/filters/lowercase_filter.h"
#include "meta/utf/utf.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view lowercase_filter::id = "lowercase";

lowercase_filter::lowercase_filter(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

lowercase_filter::lowercase_filter(const lowercase_filter& other)
    : source_{other.source_->clone()}
{
    // nothing
}

void lowercase_filter::set_content(std::string&& content)
{
    source_->set_content(std::move(content));
}

std::string lowercase_filter::next()
{
    return utf::foldcase(source_->next());
}

lowercase_filter::operator bool() const
{
    return *source_;
}
}
}
}
