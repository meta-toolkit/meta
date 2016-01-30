/**
 * @file whitespace_tokenizer.cpp
 * @author Chase Geigle
 */

#include <cassert>
#include <cctype>

#include "meta/analyzers/tokenizers/whitespace_tokenizer.h"
#include "meta/corpus/document.h"
#include "meta/io/mmap_file.h"

namespace meta
{
namespace analyzers
{
namespace tokenizers
{

const util::string_view whitespace_tokenizer::id = "whitespace-tokenizer";

whitespace_tokenizer::whitespace_tokenizer() : idx_{0}
{
}

void whitespace_tokenizer::set_content(std::string&& content)
{
    content_ = std::move(content);
    idx_ = 0;
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
    assert(!ret.empty());
    return ret;
}

whitespace_tokenizer::operator bool() const
{
    return idx_ < content_.size();
}
}
}
}
