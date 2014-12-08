/**
 * @file blank_filter.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_BLANK_FILTER_H_
#define META_BLANK_FILTER_H_

#include <memory>

#include "analyzers/filter_factory.h"
#include "util/clonable.h"
#include "util/optional.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace analyzers
{
namespace filters
{

/**
 * Filter that only retains tokens that are within a certain length range,
 * inclusive.
 */
class blank_filter : public util::clonable<token_stream, blank_filter>
{
  public:
    /**
     * Constructs a filter which rejects tokens that do not have any visible
     * characters in them.
     * @param source Where to read tokens from
     */
    blank_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     * @param other The blank_filter to copy into this one
     */
    blank_filter(const blank_filter& other);

    /**
     * Sets the content for the beginning of the filter chain.
     * @param content The string content to set
     */
    void set_content(const std::string& content) override;

    /**
     * @return the next token in the sequence
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;

    /// Identifier for this filter
    const static std::string id;

  private:
    /**
     * Advances internal state to the next valid token.
     */
    void next_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The next buffered token
    util::optional<std::string> token_;
};
}
}
}
#endif
