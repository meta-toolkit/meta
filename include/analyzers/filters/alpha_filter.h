/**
 * @file alpha_filter.h
 * @author Chase Geigle
 */

#ifndef _META_ALPHA_FILTER_H_
#define _META_ALPHA_FILTER_H_

#include "analyzers/token_stream.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Filter that removes "non-letter" characters from tokens. "Letterness" is
 * determined by the Unicode properties of each codepoint in the token.
 */
class alpha_filter : public util::clonable<token_stream, alpha_filter>
{
  public:
    /**
     * Constructs an alpha filter reading tokens from the given source.
     */
    alpha_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     */
    alpha_filter(const alpha_filter& other);

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

    /**
     * Identifier for this filter.
     */
    const static std::string id;

  private:
    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;
};
}
}

#endif
