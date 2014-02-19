/**
 * @file english_normalizer.h
 * @author Chase Geigle
 */

#ifndef _META_ENGLISH_NORMALIZER_H_
#define _META_ENGLISH_NORMALIZER_H_

#include <deque>
#include <memory>
#include "analyzers/token_stream.h"
#include "util/optional.h"

namespace meta
{
namespace analyzers
{

/**
 * Filter that normalizes english language tokens. Normalization occurs to
 * whitespace (adjacent whitespace tokens are converted to a single
 * normalized space token) and punctuation (which is split out from words
 * following basic heuristics).
 */
class english_normalizer : public token_stream
{
  public:
    /**
     * Constructs an english_normalizer which reads tokens from the given
     * source.
     */
    english_normalizer(std::unique_ptr<token_stream> source);

    /**
     * Sets the content for the beginning of the filter chain.
     */
    void set_content(const std::string& content) override;

    /**
     * Obtains the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;

  private:
    /**
     * Determines if the given token is a whitespace token.
     */
    bool is_whitespace(const std::string& token) const;

    /**
     * Converts the given non-whitespace token into a series of tokens and
     * places them on the buffer.
     */
    void parse_token(const std::string& token);

    /**
     * Checks for starting quotes in the token, adding a normalized begin
     * quote token to the stream if they exist.
     */
    uint64_t starting_quotes(uint64_t start, const std::string& token);

    /**
     * Checks if the given character is a passable quote symbol.
     */
    bool is_quote(char c);

    /**
     * Reads alphanumeric characters starting at start from the given
     * token. The first token is not checked and is assumed to be part of
     * the returned token.
     */
    uint64_t alphanum(uint64_t start, const std::string& token);

    /**
     * Returns the next buffered token.
     */
    std::string current_token();

    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;

    /**
     * Buffered tokens to return.
     */
    std::deque<std::string> tokens_;
};
}
}

#endif
