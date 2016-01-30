/**
 * @file sentence_boundary.cpp
 * @author Chase Geigle
 */

#include <fstream>

#include "meta/analyzers/filters/sentence_boundary.h"
#include "cpptoml.h"
#include "meta/io/filesystem.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

const util::string_view sentence_boundary::id = "sentence-boundary";

// static members
std::unordered_set<std::string> sentence_boundary::punc_set{};
std::unordered_set<std::string> sentence_boundary::start_exception_set{};
std::unordered_set<std::string> sentence_boundary::end_exception_set{};
bool sentence_boundary::heuristics_loaded = false;

sentence_boundary::sentence_boundary(std::unique_ptr<token_stream> source)
    : source_{std::move(source)}
{
    if (!heuristics_loaded)
        throw token_stream_exception{"heuristics must be pre-loaded"};

    tokens_.emplace_back("<s>");
}

sentence_boundary::sentence_boundary(const sentence_boundary& other)
    : source_{other.source_->clone()},
      tokens_{other.tokens_},
      prev_{other.prev_}
{
    // nothing
}

void sentence_boundary::set_content(std::string&& content)
{
    tokens_.clear();
    tokens_.emplace_back("<s>");
    prev_ = util::nullopt;
    source_->set_content(std::move(content));
}

void sentence_boundary::load_heuristics(const cpptoml::table& config)
{
    if (heuristics_loaded)
        return;

    auto punc = config.get_as<std::string>("punctuation");
    if (!punc)
        throw token_stream_exception{"configuration missing punctuation file"};

    auto start_exceptions = config.get_as<std::string>("start-exceptions");
    if (!start_exceptions)
        throw token_stream_exception{
            "configuration missing start exceptions file"};

    auto end_exceptions = config.get_as<std::string>("end-exceptions");
    if (!end_exceptions)
        throw token_stream_exception{
            "configuration missing end-exceptions file"};

    if (!filesystem::file_exists(*punc))
        throw token_stream_exception{"punctuation file does not exist: "
            + *punc};

    std::ifstream punc_file{*punc};
    std::string line;
    while (std::getline(punc_file, line))
        punc_set.emplace(std::move(line));

    if (!filesystem::file_exists(*start_exceptions))
        throw token_stream_exception{"start exceptions file does not exist: "
            + *start_exceptions};

    std::ifstream start_ex_file{*start_exceptions};
    while (std::getline(start_ex_file, line))
        start_exception_set.emplace(std::move(line));

    if (!filesystem::file_exists(*end_exceptions))
        throw token_stream_exception{"end exceptions file does not exist: "
            + *end_exceptions};

    std::ifstream end_ex_file{*end_exceptions};
    while (std::getline(end_ex_file, line))
        end_exception_set.emplace(std::move(line));

    heuristics_loaded = true;
}

std::string sentence_boundary::next()
{
    if (tokens_.empty())
    {
        // the buffer is exhausted, so we require there to be tokens available
        // in source
        if (!*source_)
            throw token_stream_exception{"next() called with empty source"};
        tokens_.emplace_back(source_->next());
    }

    if (!possible_punc(tokens_.front()) || (prev_ && !possible_end(*prev_)))
        return current_token();

    // we need to look ahead one token: if there is none, then this is
    // forced to be the end of a sentence at the end of a document
    if (!*source_)
    {
        tokens_.emplace_back("</s>");
        return current_token();
    }

    auto token = source_->next();

    // we only break sentences after whitespace
    if (token != " ")
    {
        tokens_.emplace_back(std::move(token));
        return current_token();
    }

    // we again need to look ahead a single token: if there are none, this
    // is forced to be the end of the sentence at the end of the document.
    if (!*source_)
    {
        tokens_.emplace_back("</s>");
        return current_token();
    }

    auto start_token = source_->next();
    if (!possible_start(start_token))
    {
        tokens_.emplace_back(std::move(token));
        tokens_.emplace_back(std::move(start_token));
        return current_token();
    }

    // end of sentence! add the end and start tags and the lookahead token
    // to the buffer.
    tokens_.emplace_back("</s>");
    tokens_.emplace_back(std::move(token));
    tokens_.emplace_back("<s>");
    tokens_.emplace_back(std::move(start_token));
    return current_token();
}

sentence_boundary::operator bool() const
{
    return !tokens_.empty() || *source_;
}

std::string sentence_boundary::current_token()
{
    auto token = tokens_.front();
    prev_ = token;
    tokens_.pop_front();
    return token;
}

bool sentence_boundary::possible_punc(const std::string& token)
{
    return punc_set.find(token) != punc_set.end();
}

bool sentence_boundary::possible_end(const std::string& token)
{
    return end_exception_set.find(token) == end_exception_set.end()
           && token[0] != '.';
}

bool sentence_boundary::possible_start(const std::string& token)
{
    return start_exception_set.find(token) == start_exception_set.end();
}

template <>
std::unique_ptr<token_stream>
    make_filter<sentence_boundary>(std::unique_ptr<token_stream> src,
                                   const cpptoml::table& config)
{
    sentence_boundary::load_heuristics(config);
    return make_unique<sentence_boundary>(std::move(src));
}
}
}
}
