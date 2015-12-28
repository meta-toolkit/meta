/**
 * @file ptb_normalizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_PTB_NORMALIZER_H_
#define META_PTB_NORMALIZER_H_

#include <deque>
#include <memory>
#include "meta/analyzers/token_stream.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace analyzers
{
namespace filters
{

/**
 * A filter that normalizes text to match Penn Treebank conventions. This
 * is important as a preprocessing step for input to POS taggers and
 * parsers that were trained on Penn Treebank formatted data.
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class ptb_normalizer : public util::clonable<token_stream, ptb_normalizer>
{
  public:
    /**
     * Constructs an ptb_normalizer which reads tokens from the given
     * source.
     * @param source The source to construct the filter from
     */
    ptb_normalizer(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The ptb_normalizer to copy into this one
     */
    ptb_normalizer(const ptb_normalizer& other);

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
     * @return the token from the front of the buffered tokens list
     */
    std::string current_token();

    /**
     * Performs token normalization, splitting, etc.  The token(s) are
     * placed on the token buffer.
     *
     * @param token The token to be parsed
     */
    void parse_token(const std::string& token);

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// Buffered tokens to return
    std::deque<std::string> tokens_;
};
}
}
}
#endif
