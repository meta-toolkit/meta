/**
 * @file token_stream.h
 * @author Chase Geigle
 */

#ifndef _META_TOKEN_STREAM_H_
#define _META_TOKEN_STREAM_H_

#include <string>
#include <stdexcept>

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
     * Destructor.
     */
    virtual ~token_stream() = default;

    /**
     * Basic exception class for token stream interactions.
     */
    class token_stream_exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
};
}
}
#endif
