/**
 * @file lowercase_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LOWERCASE_FILTER_H_
#define META_LOWERCASE_FILTER_H_

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
 * Filter that converts all tokens to lowercase.
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class lowercase_filter : public util::clonable<token_stream, lowercase_filter>
{
  public:
    /**
     * Constructs a new lowercase_filter, reading tokens from the given
     * source.
     * @param source The source to construct the filter from
     */
    lowercase_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The lowercase_filter to copy into this one
     */
    lowercase_filter(const lowercase_filter& other);

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
    /// The stream to read tokens from.
    std::unique_ptr<token_stream> source_;
};
}
}
}
#endif
