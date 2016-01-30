/**
 * @file english_normalizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ENGLISH_NORMALIZER_H_
#define META_ENGLISH_NORMALIZER_H_

#include <deque>
#include <memory>
#include "meta/analyzers/token_stream.h"
#include "meta/util/clonable.h"
#include "meta/util/optional.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

/**
 * Filter that normalizes English language tokens. Normalization occurs to
 * whitespace (adjacent whitespace tokens are converted to a single
 * normalized space token) and punctuation (which is split out from words
 * following basic heuristics).
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class english_normalizer
    : public util::clonable<token_stream, english_normalizer>
{
  public:
    /**
     * Constructs an english_normalizer which reads tokens from the given
     * source.
     * @param source The source to construct the filter from
     */
    english_normalizer(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The english_normalizer to copy into this one
     */
    english_normalizer(const english_normalizer& other);

    /**
     * Sets the content for the beginning of the filter chain.
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * Obtains the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;

    /// Identifier for this filter
    const static util::string_view id;

  private:
    /**
     * Determines if the given token is a whitespace token.
     * @param token The given token
     */
    bool is_whitespace(const std::string& token) const;

    /**
     * Converts the given non-whitespace token into a series of tokens and
     * places them on the buffer.
     * @param token The given token
     */
    void parse_token(const std::string& token);

    /**
     * Checks for starting quotes in the token, adding a normalized begin
     * quote token to the stream if they exist.
     * @param start The index to start searching at
     * @param token The given token
     */
    uint64_t starting_quotes(uint64_t start, const std::string& token);

    /**
     * Checks if the given character is a passable quote symbol.
     * @param c The given character
     */
    bool is_quote(char c);

    /**
     * Reads consecutive dash characters.
     * @param start The index to start searching at
     * @param token The given token
     */
    uint64_t strip_dashes(uint64_t start, const std::string& token);

    /**
     * Reads "word" characters (alpha numeric and dashes) starting at start
     * from the given token. The first token is not checked and is assumed
     * to be part of the returned token.
     * @param start The index to start searching at
     * @param token The given token
     */
    uint64_t word(uint64_t start, const std::string& token);

    /**
     * @return the next buffered token.
     */
    std::string current_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// Buffered tokens to return
    std::deque<std::string> tokens_;
};
}
}
}
#endif
