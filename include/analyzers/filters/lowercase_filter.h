/**
 * @file lowercase_filter.h
 * @author Chase Geigle
 */

#ifndef _META_LOWERCASE_FILTER_H_
#define _META_LOWERCASE_FILTER_H_

#include <memory>
#include "analyzers/token_stream.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Filter that converts all tokens to lowercase.
 */
class lowercase_filter : public util::clonable<token_stream, lowercase_filter>
{
  public:
    /**
     * Constructs a new lowercase_filter, reading tokens from the given
     * source.
     */
    lowercase_filter(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     */
    lowercase_filter(const lowercase_filter& other);

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
     * The stream to read tokens from.
     */
    std::unique_ptr<token_stream> source_;
};
}
}

#endif
