/**
 * @file whitespace_tokenizer.cpp
 * @author Chase Geigle
 */

#include <cctype>

#include "analyzers/tokenizers/whitespace_tokenizer.h"
#include "corpus/document.h"
#include "io/mmap_file.h"

namespace meta
{
namespace analyzers
{

whitespace_tokenizer::whitespace_tokenizer(corpus::document& doc) : idx_{0}
{
    if (doc.contains_content())
        content_ = doc.content();
    else
    {
        io::mmap_file file{doc.path()};
        content_ = {file.begin(), file.begin() + file.size()};
    }
}

std::string whitespace_tokenizer::next()
{
    if (!*this)
        throw token_stream_exception{"next() called with no tokens left"};

    std::string ret;
    // all whitespace chars are their own token
    if (std::isspace(content_[idx_]))
    {
        ret.push_back(content_[idx_++]);
    }
    // otherwise, concatenate all non-whitespace chars together until we
    // find a whitespace char
    else
    {
        while (*this && !std::isspace(content_[idx_]))
            ret.push_back(content_[idx_++]);
    }
    return ret;
}

whitespace_tokenizer::operator bool() const
{
    return idx_ < content_.size();
}
}
}
