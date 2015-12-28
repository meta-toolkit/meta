/**
 * @file english_normalizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <cctype>
#include "meta/analyzers/filters/english_normalizer.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view english_normalizer::id = "english-normalizer";

english_normalizer::english_normalizer(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

english_normalizer::english_normalizer(const english_normalizer& other)
    : source_{other.source_->clone()}, tokens_{other.tokens_}
{
    // nothing
}

void english_normalizer::set_content(std::string&& content)
{
    tokens_.clear();
    source_->set_content(std::move(content));
}

std::string english_normalizer::next()
{
    // if we have buffered tokens, keep returning them until we have
    // exhausted the buffer
    if (!tokens_.empty())
        return current_token();

    if (!*source_)
        throw token_stream_exception{"next() called with empty source"};

    auto token = source_->next();

    // if we have a whitespace token, keep reading any following whitespace
    // tokens to collapse them down to a single space. token_, afterwards,
    // will contain the next non-whitespace token for the next call to
    // next().
    if (is_whitespace(token))
    {
        while (is_whitespace(token) && *source_)
            token = source_->next();
        if (!is_whitespace(token)) // source_ was non-empty after whitespace
            parse_token(token);
        return " ";
    }

    parse_token(token);
    return current_token();
}

english_normalizer::operator bool() const
{
    return !tokens_.empty() || *source_;
}

bool english_normalizer::is_whitespace(const std::string& token) const
{
    auto space = [](char c) { return std::isspace(c); };
    return std::all_of(token.begin(), token.end(), space);
}

void english_normalizer::parse_token(const std::string& token)
{
    if (token.length() < 2)
    {
        tokens_.push_back(token);
        return;
    }

    uint64_t idx = 0;
    uint64_t end = token.length();

    // check for ending quotation marks: if they exist, don't parse that
    // part of the string and remember that we need to add the end quotes
    // token after parsing the rest of the token
    bool end_quotes = false;
    if (token[end - 1] == '"')
    {
        end = end - 1;
        end_quotes = true;
    }
    else if (is_quote(token[end - 1]) && is_quote(token[end - 2]))
    {
        end = end - 2;
        end_quotes = true;
    }

    idx = starting_quotes(idx, token);

    // other leading punctuation should be separate tokens
    while (idx < end && !std::isalnum(token[idx]))
        tokens_.emplace_back(1, token[idx++]);

    // split out sequences of alphanumeric characters into separate tokens
    while (idx < end)
        idx = word(idx, token);

    if (end_quotes)
        tokens_.push_back("''");
}

uint64_t english_normalizer::starting_quotes(uint64_t start,
                                             const std::string& token)
{
    if (token[start] == '"')
    {
        tokens_.push_back("``");
        return start + 1;
    }

    if (is_quote(token[start]) && is_quote(token[start + 1]))
    {
        tokens_.push_back("``");
        return start + 2;
    }

    return start;
}

bool english_normalizer::is_quote(char c)
{
    return c == '\'' || c == '`';
}

uint64_t english_normalizer::strip_dashes(uint64_t start,
                                          const std::string& token)
{
    auto idx = start + 1;
    while (idx < token.length() && token[idx] == '-')
        ++idx;
    tokens_.emplace_back(token, start, idx - start);
    return idx;
}

uint64_t english_normalizer::word(uint64_t start, const std::string& token)
{
    // special case leading dashes: if there are consecutive ones, we want
    // to strip them out into their own token
    if (token[start] == '-' && start + 1 < token.length() && token[start + 1]
                                                             == '-')
        start = strip_dashes(start, token);

    uint64_t idx = start + 1; // the token is at least one character, which may
                              // have been a punctuation marker from the
                              // previous pass
    while (idx < token.length())
    {
        if (token[idx] == '-' && idx + 1 < token.length() && token[idx + 1]
                                                             == '-')
        {
            // multiple dashes in a row---like this, indicating an em dash
            // or something

            // place the current token, before the dash, into the buffer
            tokens_.emplace_back(token, start, idx - start);
            // place the dashes onto the buffer
            start = strip_dashes(idx, token);
        }

        // stop at first punctuation that is not a dash (we want to keep
        // words like "forty-five")
        if (std::ispunct(token[idx]) && token[idx] != '-')
            break;
        ++idx;
    }

    tokens_.emplace_back(token, start, idx - start);
    return idx;
}

std::string english_normalizer::current_token()
{
    auto token = tokens_.front();
    tokens_.pop_front();
    return token;
}
}
}
}
