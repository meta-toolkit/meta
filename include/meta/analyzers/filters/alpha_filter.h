/**
 * @file alpha_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ALPHA_FILTER_H_
#define META_ALPHA_FILTER_H_

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
 * Filter that removes "non-letter" characters from tokens. "Letterness" is
 * determined by the Unicode properties of each codepoint in the token.
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class alpha_filter : public util::clonable<token_stream, alpha_filter>
{
  public:
    /**
     * Constructs an alpha filter reading tokens from the given source.
     * @param source The source to construct the filter from
     */
    alpha_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The alpha_filter to copy into this one
     */
    alpha_filter(const alpha_filter& other);

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
     * Finds the next valid token for this filter.
     */
    void next_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The buffered token.
    util::optional<std::string> token_;
};
}
}
}
#endif
