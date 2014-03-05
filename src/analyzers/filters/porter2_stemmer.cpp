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

const std::string porter2_stemmer::id = "porter2-stemmer";

porter2_stemmer::porter2_stemmer(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

porter2_stemmer::porter2_stemmer(const porter2_stemmer& other)
    : source_{other.source_->clone()}
{
    // nothing
}

void porter2_stemmer::set_content(const std::string& content)
{
    source_->set_content(content);
}

std::string porter2_stemmer::next()
{
    auto tok = source_->next();
    Porter2Stemmer::stem(tok);
    return tok;
}

porter2_stemmer::operator bool() const
{
    return *source_;
}

}
}
