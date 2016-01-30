/**
 * @file porter2_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_FILTER_PORTER2_FILTER_H_
#define META_FILTER_PORTER2_FILTER_H_

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
 * Filter that stems words according to the porter2 stemmer algorithm.
 * Requires that the porter2 stemmer project submodule be downloaded.
 *
 * Required config parameters: none.
 * Optional config parameters: none.
 */
class porter2_filter : public util::clonable<token_stream, porter2_filter>
{
  public:
    /**
     * Constructs a new porter2 stemmer filter, reading tokens from
     * the given source.
     * @param source The source to construct the filter from
     */
    porter2_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The porter2_filter to copy into this one
     */
    porter2_filter(const porter2_filter& other);

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
     * Determines if there are more tokens available in the stream.
     */
    operator bool() const override;

    /// Identifier for this filter
    const static util::string_view id;

  private:
    /**
     * Finds the next valid token for this filter.
     */
    void next_token();

    /// The stream to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The buffered next token.
    util::optional<std::string> token_;
};
}
}
}
#endif
