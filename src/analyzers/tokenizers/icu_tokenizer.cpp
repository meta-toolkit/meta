/**
 * @file icu_tokenizer.cpp
 * @author Chase Geigle
 */

#include <algorithm>
#include <deque>

#include "cpptoml.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/utf/segmenter.h"
#include "meta/utf/utf.h"
#include "meta/util/pimpl.tcc"

namespace meta
{
namespace analyzers
{
namespace tokenizers
{

const util::string_view icu_tokenizer::id = "icu-tokenizer";

/**
 * Implementation class for the icu_tokenizer.
 */
class icu_tokenizer::impl
{
  public:
    impl(bool suppress_tags) : suppress_tags_{suppress_tags}
    {
        // nothing
    }

    explicit impl(utf::segmenter segmenter, bool suppress_tags)
        : suppress_tags_{suppress_tags}, segmenter_{std::move(segmenter)}
    {
        // nothing
    }

    /**
     * @param content The string content to set
     * TODO: can we make this be a streaming API instead of buffering all
     * of the tokens?
     */
    void set_content(std::string content)
    {
        auto pred = [](char c) {
            return c == '\n' || c == '\v' || c == '\f' || c == '\r';
        };
        // doing this because the sentence segmenter gets confused by
        // newlines appearing within a pargraph. Plus, we don't really care
        // about the kind of whitespace that was used for IR tasks.
        std::replace_if(content.begin(), content.end(), pred, ' ');

        segmenter_.set_content(content);
        for (const auto& sentence : segmenter_.sentences())
        {
            if (!suppress_tags_)
                tokens_.emplace_back("<s>");
            for (const auto& word : segmenter_.words(sentence))
            {
                auto wrd = segmenter_.content(word);
                if (wrd.empty())
                    continue;

                // check first character, if it's whitespace skip it
                int32_t i = 0;
                auto length = static_cast<int32_t>(wrd.size());
                auto codepoint
                    = utf::detail::utf8_next_codepoint(wrd.data(), i, length);
                if (codepoint < 0
                    || utf::isspace(static_cast<uint32_t>(codepoint)))
                    continue;

                tokens_.emplace_back(wrd.to_string());
            }
            if (!suppress_tags_)
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
        auto result = std::move(tokens_.front());
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
    /// Whether or not to suppress "<s>" or "</s>" generation
    const bool suppress_tags_;

    /// UTF segmenter to use for this tokenizer
    utf::segmenter segmenter_;

    /// Buffered tokens
    std::deque<std::string> tokens_;
};

icu_tokenizer::icu_tokenizer(bool suppress_tags) : impl_{suppress_tags}
{
    // nothing
}

icu_tokenizer::icu_tokenizer(utf::segmenter segmenter, bool suppress_tags)
    : impl_{std::move(segmenter), suppress_tags}
{
    // nothing
}

icu_tokenizer::icu_tokenizer(icu_tokenizer&&) = default;

icu_tokenizer::icu_tokenizer(const icu_tokenizer& other) : impl_{*other.impl_}
{
    // nothing
}

icu_tokenizer::~icu_tokenizer() = default;

void icu_tokenizer::set_content(std::string&& content)
{
    impl_->set_content(std::move(content));
}

std::string icu_tokenizer::next()
{
    return impl_->next();
}

icu_tokenizer::operator bool() const
{
    return static_cast<bool>(*impl_);
}

template <>
std::unique_ptr<token_stream>
make_tokenizer<icu_tokenizer>(const cpptoml::table& config)
{
    auto language = config.get_as<std::string>("language");
    auto country = config.get_as<std::string>("country");
    bool suppress_tags = config.get_as<bool>("suppress-tags").value_or(false);

    if (country && !language)
        throw token_stream_exception{
            "icu_tokenizer cannot be created with just a country"};

    if (language)
    {
        if (country)
            return make_unique<icu_tokenizer>(
                utf::segmenter{*language, *country}, suppress_tags);
        else
            return make_unique<icu_tokenizer>(utf::segmenter{*language},
                                              suppress_tags);
    }

    return make_unique<icu_tokenizer>(suppress_tags);
}
}
}
}
