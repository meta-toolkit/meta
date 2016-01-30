/**
 * @file empty_sentence_filter.cpp
 * @author Chase Geigle
 */

#include "meta/analyzers/filters/empty_sentence_filter.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view empty_sentence_filter::id = "empty-sentence";

empty_sentence_filter::empty_sentence_filter(
    std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    next_token();
}

empty_sentence_filter::empty_sentence_filter(const empty_sentence_filter& other)
    : source_{other.source_->clone()},
      first_{other.first_},
      second_{other.second_}
{
    // nothing
}

void empty_sentence_filter::set_content(std::string&& content)
{
    source_->set_content(std::move(content));
    first_ = second_ = util::nullopt;
    next_token();
}

void empty_sentence_filter::next_token()
{
    if (second_ || !*source_)
    {
        first_ = second_;
        second_ = util::nullopt;
        return;
    }

    while (*source_)
    {
        first_ = source_->next();
        if (!*source_ || *first_ != "<s>")
            return;
        second_ = source_->next();
        if (*second_ != "</s>")
            return;
        first_ = second_ = util::nullopt;
    }
}

std::string empty_sentence_filter::next()
{
    auto tok = *first_;
    next_token();
    return tok;
}

empty_sentence_filter::operator bool() const
{
    return static_cast<bool>(first_);
}
}
}
}
