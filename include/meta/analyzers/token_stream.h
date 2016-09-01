/**
 * @file token_stream.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TOKEN_STREAM_H_
#define META_TOKEN_STREAM_H_

#include <memory>
#include <string>
#include <stdexcept>

#include "meta/config.h"

namespace meta
{
namespace analyzers
{

/**
 * Base class that represents a stream of tokens that have been extracted
 * from a document. These tokens may be raw tokens (in the case of a
 * tokenizer class) or filtered tokens (from the filter classes).
 */
class token_stream
{
  public:
    /**
     * Obtains the next token in the sequence.
     */
    virtual std::string next() = 0;

    /**
     * Determines whether there are more tokens available in the
     * stream.
     */
    virtual operator bool() const = 0;

    /**
     * Sets the content for the stream.
     * @param content The string content to set
     */
    virtual void set_content(std::string&& content) = 0;

    /**
     * Destructor.
     */
    virtual ~token_stream() = default;

    /**
     * Clones the given token stream.
     * @return a unique_ptr to copy this object
     */
    virtual std::unique_ptr<token_stream> clone() const = 0;
};

/**
 * Basic exception class for token stream interactions.
 */
class token_stream_exception : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
}
#endif
