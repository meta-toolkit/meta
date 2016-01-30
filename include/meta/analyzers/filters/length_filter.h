/**
 * @file length_filter.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_LENGTH_FILTER_H_
#define META_LENGTH_FILTER_H_

#include <memory>

#include "meta/analyzers/filter_factory.h"
#include "meta/util/clonable.h"
#include "meta/util/optional.h"

namespace cpptoml
{
class table;
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
 *
 * Required config parameters:
 * ~~~toml
 * min = 2  # any integer
 * max = 32 # any integer >= min
 * ~~~
 *
 * Optional config parameters: none.
 */
class length_filter : public util::clonable<token_stream, length_filter>
{
  public:
    /**
     * Constructs a length filter, reading tokens from the given source
     * and eliminating any that are shorter than min characters in length
     * or longer than max characters in length.
     * @param source Where to read tokens from
     * @param min The minimum token length
     * @param max The maximum token length
     */
    length_filter(std::unique_ptr<token_stream> source, uint64_t min,
                  uint64_t max);

    /**
     * Copy constructor.
     * @param other The length_filter to copy into this one
     */
    length_filter(const length_filter& other);

    /**
     * Sets the content for the beginning of the filter chain.
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * @return the next token in the sequence
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
     * Advances internal state to the next valid token.
     */
    void next_token();

    /// The source to read tokens from
    std::unique_ptr<token_stream> source_;

    /// The next buffered token
    util::optional<std::string> token_;

    /// The minimum length of a token that can be emitted by this filter
    uint64_t min_length_;

    /// The maximum length of a token that can be emitted by this filter
    uint64_t max_length_;
};

/**
 * Specialization of the factory method for creating length_filters.
 */
template <>
std::unique_ptr<token_stream>
    make_filter<length_filter>(std::unique_ptr<token_stream>,
                               const cpptoml::table&);
}
}
}
#endif
