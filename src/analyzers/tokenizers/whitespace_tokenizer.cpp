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

whitespace_tokenizer::whitespace_tokenizer(bool suppress_whitespace)
    : suppress_whitespace_{suppress_whitespace}
{
    // nothing
}

void whitespace_tokenizer::set_content(std::string&& content)
{
    content_ = std::move(content);
    it_ = content_.begin();
    if (suppress_whitespace_)
        consume_adjacent_whitespace();
}

void whitespace_tokenizer::consume_adjacent_whitespace()
{
    it_ = std::find_if_not(it_, content_.cend(), [](char c) {
        return std::isspace(c);
    });
}

std::string whitespace_tokenizer::next()
{
    if (!*this)
        throw token_stream_exception{"next() called with no tokens left"};

    if (std::isspace(*it_))
    {
        if (suppress_whitespace_)
        {
            consume_adjacent_whitespace();
        }
        else
        {
            // all whitespace chars are their own token
            return std::string(1, *it_++);
        }
    }

    // otherwise, find the next whitespace character and emit the sequence
    // of consecutive non-whitespace characters as a token
    auto begin = it_;
    it_ = std::find_if(it_, content_.cend(), [](char c) {
        return std::isspace(c);
    });
    std::string ret{begin, it_};
    assert(!ret.empty());

    if (suppress_whitespace_)
        consume_adjacent_whitespace();

    return ret;
}

whitespace_tokenizer::operator bool() const
{
    return !content_.empty() && it_ != content_.cend();
}

template <>
std::unique_ptr<token_stream>
make_tokenizer<whitespace_tokenizer>(const cpptoml::table& config)
{
    auto suppress_whitespace
        = config.get_as<bool>("suppress-whitespace").value_or(true);

    return make_unique<whitespace_tokenizer>(suppress_whitespace);
}
}
}
}
