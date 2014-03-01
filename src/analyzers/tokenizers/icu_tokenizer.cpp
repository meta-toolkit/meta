/**
 * @file icu_tokenizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <deque>

#include "analyzers/tokenizers/icu_tokenizer.h"
#include "util/pimpl.tcc"
#include "util/utf.h"

namespace meta
{
namespace analyzers
{

/**
 * Implementation class for the icu_tokenizer.
 */
class icu_tokenizer::impl
{
  public:
    // TODO: can we make this be a streaming API instead of buffering all
    // of the tokens?
    void set_content(std::string content)
    {
        auto pred = [](char c)
        {
            return c == '\n' || c == '\v' || c == '\f' || c == '\r';
        };
        // doing this because the sentence segmenter gets confused by
        // newlines appearing within a pargraph. Plus, we don't really care
        // about the kind of whitespace that was used for IR tasks.
        std::replace_if(content.begin(), content.end(), pred, ' ');

        utf::segmenter segmenter;
        segmenter.set_content(content);
        for (const auto& sentence : segmenter.sentences())
        {
            tokens_.emplace_back("<s>");
            for (const auto& word : segmenter.words(sentence))
                tokens_.emplace_back(segmenter.content(word));
            tokens_.emplace_back("</s>");
        }
    }

    std::string next()
    {
        if (!*this)
            throw token_stream_exception{"next() called with no tokens left"};
        auto result = tokens_.front();
        tokens_.pop_front();
        return result;
    }

    explicit operator bool() const
    {
        return !tokens_.empty();
    }
  private:
    std::deque<std::string> tokens_;
};

icu_tokenizer::icu_tokenizer() = default;
icu_tokenizer::icu_tokenizer(icu_tokenizer&&) = default;

icu_tokenizer::icu_tokenizer(const icu_tokenizer& other)
    : impl_{*other.impl_}
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
