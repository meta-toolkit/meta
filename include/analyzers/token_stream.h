/**
 * @file token_stream.h
 * @author Chase Geigle
 */

#ifndef _META_TOKEN_STREAM_H_
#define _META_TOKEN_STREAM_H_

#include <memory>
#include <string>
#include <stdexcept>

#include "util/shim.h"

namespace cpptoml {
class toml_group;
}

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
     */
    virtual void set_content(const std::string& content) = 0;

    /**
     * Destructor.
     */
    virtual ~token_stream() = default;

    /**
     * Clones the given token stream.
     */
    virtual std::unique_ptr<token_stream> clone() const = 0;

    /**
     * Basic exception class for token stream interactions.
     */
    class token_stream_exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };
};

/**
 * Factory method for creating a tokenizer. This should be specialized if
 * your given tokenizer requires special construction behavior.
 */
template <class Tokenizer>
std::unique_ptr<token_stream> make_tokenizer(const cpptoml::toml_group&)
{
    return make_unique<Tokenizer>();
}

/**
 * Factory method for creating a filter. This should be specialized if your
 * given filter requires special behavior.
 */
template <class Filter>
std::unique_ptr<token_stream> make_filter(std::unique_ptr<token_stream> source,
                                          const cpptoml::toml_group&)
{
    return make_unique<Filter>(std::move(source));
}


}
}
#endif
