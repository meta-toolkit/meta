/**
 * @file icu_tokenizer.h
 * @author Chase Geigle
 */

#ifndef _META_ICU_TOKENIZER_H_
#define _META_ICU_TOKENIZER_H_

#include "analyzers/token_stream.h"
#include "util/clonable.h"
#include "util/pimpl.h"

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
 * Converts documents into streams of tokens by following the unicode
 * standards for sentence and word segmentation.
 */
class icu_tokenizer : public util::clonable<token_stream, icu_tokenizer>
{
  public:
    /**
     * Creates an icu_tokenizer.
     */
    icu_tokenizer();

    /**
     * Copies an icu_tokenizer.
     */
    icu_tokenizer(const icu_tokenizer& other);

    /**
     * Moves an icu_tokenizer.
     */
    icu_tokenizer(icu_tokenizer&& other);

    /**
     * Destroys an icu_tokenizer.
     */
    ~icu_tokenizer();

    /**
     * Sets the content for the tokenizer to parse. This input is
     * assumed to be utf-8 encoded. It will be converted to utf-16
     * internally by ICU for the segmentation, but all tokens are
     * output as utf-8 encoded strings.
     */
    void set_content(const std::string& content) override;

    /**
     * Obtains the next token in the document. This will either by a
     * sentence boundary ("<s>" or "</s>"), a token consisting of
     * non-whitespace characters, or a token consisting of only
     * whitespace characters.
     */
    std::string next() override;

    /**
     * Determines if there are more tokens in the document.
     */
    operator bool() const override;

  private:
    class impl;
    util::pimpl<impl> impl_;
};
}
}
#endif
