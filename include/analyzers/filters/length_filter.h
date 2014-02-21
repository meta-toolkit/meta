/**
 * @file length_filter.h
 * @author Chase Geigle
 */

#ifndef _META_LENGTH_FILTER_H_
#define _META_LENGTH_FILTER_H_

#include <memory>

#include "analyzers/token_stream.h"
#include "util/clonable.h"
#include "util/optional.h"

namespace meta
{
namespace analyzers
{

/**
 * Filter that only retains tokens that are within a certain length range,
 * inclusive.
 */
class length_filter : public util::clonable<token_stream, length_filter>
{
  public:
    /**
     * Constructs a length filter, reading tokens from the given source
     * and eliminating any that are shorter than min characters in length
     * or longer than max characters in length.
     */
    length_filter(std::unique_ptr<token_stream> source, uint64_t min,
                  uint64_t max);

    /**
     * Copy constructor.
     */
    length_filter(const length_filter& other);

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
     * Advances internal state to the next valid token.
     */
    void next_token();

    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;

    /**
     * The next buffered token.
     */
    util::optional<std::string> token_;

    /**
     * The minimum length of a token that can be emitted by this filter.
     */
    uint64_t min_length_;

    /**
     * The maximum length of a token that can be emitted by this filter.
     */
    uint64_t max_length_;
};
}
}
#endif
