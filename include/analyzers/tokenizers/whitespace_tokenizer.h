/**
 * @file whitespace_tokenizer.h
 * @author Chase Geigle
 */

#ifndef _META_WHITESPACE_TOKENIZER_H_
#define _META_WHITESPACE_TOKENIZER_H_

#include "analyzers/token_stream.h"

namespace meta
{
namespace corpus
{
class document;
}
}

namespace meta
{
namespace analyzers
{

/**
 * Converts documents into streams of whitespace delimited tokens. This
 * tokenizer preserves the whitespace, but combines adjacent non-whitespace
 * characters together into individual tokens.
 */
class whitespace_tokenizer : public token_stream
{
    public:
        /**
         * Creates a whitespace_tokenizer operating on the given document.
         */
        whitespace_tokenizer(corpus::document& doc);

        /**
         * Obtains the next token in the document. This will either be a
         * whitespace character, or a token consisting of a sequence of
         * non-whitespace characters.
         */
        std::string next() override;

        /**
         * Determines if there are more tokens in the document.
         */
        operator bool() const override;

    private:
        std::string content_;
        uint64_t idx_;
};
}
}
#endif
