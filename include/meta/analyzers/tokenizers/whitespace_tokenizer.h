/**
 * @file whitespace_tokenizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_WHITESPACE_TOKENIZER_H_
#define META_WHITESPACE_TOKENIZER_H_

#include "meta/analyzers/filter_factory.h"
#include "meta/analyzers/token_stream.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace corpus
{
class document;
}
}

namespace meta
{
namespace analyzers
{
namespace tokenizers
{

/**
 * Converts documents into streams of whitespace delimited tokens. This
 * tokenizer preserves the whitespace, but combines adjacent non-whitespace
 * characters together into individual tokens.
 */
class whitespace_tokenizer : public util::clonable<token_stream,
                                                   whitespace_tokenizer>
{
  public:
    /**
     * Creates a whitespace_tokenizer.
     * @param suppress_whitespace Whether to suppress whitespace tokens
     * themselves or not.
     */
    whitespace_tokenizer(bool suppress_whitespace = true);

    /**
     * Sets the content for the tokenizer to parse.
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * @return the next token in the document. This will either be a
     * whitespace character, or a token consisting of a sequence of
     * non-whitespace characters.
     */
    std::string next() override;

    /**
     * Determines if there are more tokens in the document.
     */
    operator bool() const override;

    /// Identifier for this tokenizer
    const static util::string_view id;

  private:
    void consume_adjacent_whitespace();

    /// Buffered string content for this tokenizer
    std::string content_;

    /// Whether or not to output whitespace tokens
    const bool suppress_whitespace_;

    /// Character iterator into the current buffer
    std::string::const_iterator it_;
};

/**
 * Specialization of the factory method use to create whitespace_tokenizers.
 */
template <>
std::unique_ptr<token_stream>
    make_tokenizer<whitespace_tokenizer>(const cpptoml::table& config);
}
}
}
#endif
