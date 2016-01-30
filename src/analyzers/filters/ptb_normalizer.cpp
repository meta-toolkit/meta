/**
 * @file ptb_normalizer.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <algorithm>
#include "meta/analyzers/filters/ptb_normalizer.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view ptb_normalizer::id = "ptb-normalizer";

ptb_normalizer::ptb_normalizer(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    // nothing
}

ptb_normalizer::ptb_normalizer(const ptb_normalizer& other)
    : source_{other.source_->clone()}, tokens_{other.tokens_}
{
    // nothing
}

void ptb_normalizer::set_content(std::string&& content)
{
    tokens_.clear();
    source_->set_content(std::move(content));
}

std::string ptb_normalizer::next()
{
    // if we have buffered tokens, keep returning them until we have
    // exhausted the buffer
    if (!tokens_.empty())
    {
        auto token = tokens_.front();
        tokens_.pop_front();
        return token;
    }

    if (!*source_)
        throw token_stream_exception{"next() called with empty source"};

    auto token = source_->next();

    if (token == "\"")
    {
        tokens_.push_back("``");

        // keep reading until we either hit the next pair of quotes or, in
        // error cases, the end of sentence marker. Buffer tokens along the
        // way.
        while (*source_)
        {
            auto nxt = source_->next();
            if (nxt == "\"")
            {
                tokens_.push_back("''");
                return current_token();
            }

            parse_token(nxt);

            if (nxt == "</s>")
                return current_token();
        }

        // only get here if we've parsed the whole source and never found a
        // matching end quote, so just return the buffered tokens at this
        // point
        return current_token();
    }

    parse_token(token);
    return current_token();
}

ptb_normalizer::operator bool() const
{
    return !tokens_.empty() || (*source_);
}

std::string ptb_normalizer::current_token()
{
    auto token = tokens_.front();
    tokens_.pop_front();
    return token;
}

void ptb_normalizer::parse_token(const std::string& token)
{
    /// @see http://www.cis.upenn.edu/~treebank/tokenizer.sed
    if (token == "(")
    {
        tokens_.push_back("-LRB-");
        return;
    }
    else if (token == ")")
    {
        tokens_.push_back("-RRB-");
        return;
    }
    else if (token == "[")
    {
        tokens_.push_back("-LSB-");
        return;
    }
    else if (token == "]")
    {
        tokens_.push_back("-RSB-");
        return;
    }
    else if (token == "{")
    {
        tokens_.push_back("-LCB-");
        return;
    }
    else if (token == "}")
    {
        tokens_.push_back("-RCB-");
        return;
    }
    else if (token.find("'") != token.npos)
    {
        auto apos = token.find("'");

        // possessive or close single quote
        if (apos == token.length() - 1 && token.length() > 1)
        {
            tokens_.push_back(token.substr(0, apos));
            tokens_.push_back(token.substr(apos));
            return;
        }
        else if (apos + 1 == token.length() - 1)
        {
            auto after = std::tolower(token.at(apos + 1));

            // as in it's, I'm, we'd
            if (after == 's' || after == 'm' || after == 'd')
            {
                tokens_.push_back(token.substr(0, apos));
                tokens_.push_back(token.substr(apos));
                return;
            }
            // n't
            else if (after == 't' && apos != 0
                     && std::tolower(token.at(apos - 1)) == 'n')
            {
                tokens_.push_back(token.substr(0, apos - 1));
                tokens_.push_back(token.substr(apos - 1));
                return;
            }
        }
        else if (apos + 2 == token.length() - 1)
        {
            auto after1 = std::tolower(token.at(apos + 1));
            auto after2 = std::tolower(token.at(apos + 2));

            if ((after1 == 'l' && after2 == 'l')
                || ((after1 == 'r' || after1 == 'v') && after2 == 'e'))
            {
                tokens_.push_back(token.substr(0, apos));
                tokens_.push_back(token.substr(apos));
                return;
            }
        }
    }

    auto lower = token;
    std::transform(token.begin(), token.end(), lower.begin(), [](char c)
                   {
        return std::tolower(c);
    });

    if (lower.find("d'ye") != lower.npos)
    {
        tokens_.push_back(token.substr(0, token.find("'") + 1));
        tokens_.push_back("ye");
        return;
    }
    else if (lower == "more'n")
    {
        tokens_.push_back(token.substr(0, token.find("'")));
        tokens_.push_back("'n");
        return;
    }
    else if (token == "'" && *source_)
    {
        auto nxt = source_->next();
        auto lnxt = nxt;
        std::transform(lnxt.begin(), lnxt.end(), lnxt.begin(), [](char c)
                       {
            return std::tolower(c);
        });

        if (lnxt == "twas" || lnxt == "tis")
        {
            std::string apot = "'";
            apot += nxt.at(0);
            tokens_.push_back(apot);
            tokens_.push_back(nxt.substr(1));
            return;
        }

        tokens_.push_back(token);
        parse_token(nxt);
        return;
    }
    else if (lower == "cannot" || lower == "gimme" || lower == "gonna"
             || lower == "lemme" || lower == "wanna")
    {
        tokens_.push_back(token.substr(0, 3));
        tokens_.push_back(token.substr(3));
        return;
    }

    // if all checks passed through, then leave the token alone
    tokens_.push_back(token);
}
}
}
}
