/**
 * @file empty_sentence_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_EMPTY_SENTENCE_FILTER_H_
#define META_EMPTY_SENTENCE_FILTER_H_

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
 * Filter that removes any empty sentences from the token stream. Empty
 * sentences can be caused by filters in the filter chain that follow
 * sentence boundary detection.
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class empty_sentence_filter
    : public util::clonable<token_stream, empty_sentence_filter>
{
  public:
    /**
     * Constructs an empty_sentence_filter which reads tokens from the
     * given source.
     * @param source The source to construct the filter from
     */
    empty_sentence_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The empty_sentence_filter to copy into this one
     */
    empty_sentence_filter(const empty_sentence_filter& other);

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

    /// Keeps track of the left hand side of a potentially empty sentence
    util::optional<std::string> first_;

    /// Keeps track of the right hand side of a potentially empty sentence
    util::optional<std::string> second_;
};
}
}
}
#endif
