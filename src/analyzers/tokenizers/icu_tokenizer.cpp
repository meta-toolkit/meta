/**
 * @file icu_tokenizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <deque>

#include <unicode/utf.h>
#include <unicode/uchar.h>

#include "analyzers/tokenizers/icu_tokenizer.h"
#include "util/pimpl.tcc"
#include "utf/segmenter.h"

namespace meta
{
namespace analyzers
{
namespace tokenizers
{

const std::string icu_tokenizer::id = "icu-tokenizer";

/**
 * Implementation class for the icu_tokenizer.
 */
class icu_tokenizer::impl
{
  public:
    /**
     * @param content The string content to set
     * TODO: can we make this be a streaming API instead of buffering all
     * of the tokens?
     */
    void set_content(std::string content)
    {
        auto pred = [](char c)
        { return c == '\n' || c == '\v' || c == '\f' || c == '\r'; };
        // doing this because the sentence segmenter gets confused by
        // newlines appearing within a pargraph. Plus, we don't really care
        // about the kind of whitespace that was used for IR tasks.
        std::replace_if(content.begin(), content.end(), pred, ' ');

        segmenter_.set_content(content);
        for (const auto& sentence : segmenter_.sentences())
        {
            tokens_.emplace_back("<s>");
            for (const auto& word : segmenter_.words(sentence))
            {
                auto wrd = segmenter_.content(word);
                if (wrd.empty())
                    continue;

                // check first character, if it's whitespace skip it
                UChar32 codepoint;
                U8_GET_UNSAFE(wrd.c_str(), 0, codepoint);
                if (u_isUWhiteSpace(codepoint))
                  continue;

                tokens_.emplace_back(std::move(wrd));
            }
            tokens_.emplace_back("</s>");
        }
    }

    /**
     * @return the next token
     */
    std::string next()
    {
        if (!*this)
            throw token_stream_exception{"next() called with no tokens left"};
        auto result = tokens_.front();
        tokens_.pop_front();
        return result;
    }

    /**
     * True if tokens is not empty.
     */
    explicit operator bool() const
    {
        return !tokens_.empty();
    }

  private:
    /// UTF segmenter to use for this tokenizer
    utf::segmenter segmenter_;

    /// Buffered tokens
    std::deque<std::string> tokens_;
};

icu_tokenizer::icu_tokenizer() = default;
icu_tokenizer::icu_tokenizer(icu_tokenizer&&) = default;

icu_tokenizer::icu_tokenizer(const icu_tokenizer& other) : impl_{*other.impl_}
{
}

icu_tokenizer::~icu_tokenizer() = default;

void icu_tokenizer::set_content(const std::string& content)
{
    impl_->set_content(content);
}

std::string icu_tokenizer::next()
{
    return impl_->next();
}

icu_tokenizer::operator bool() const
{
    return static_cast<bool>(*impl_);
}
}
}
}
