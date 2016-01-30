/**
 * @file character_tokenizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_CHARACTER_TOKENIZER_H_
#define META_CHARACTER_TOKENIZER_H_

#include "meta/analyzers/token_stream.h"
#include "meta/util/clonable.h"
#include "meta/util/string_view.h"

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
namespace tokenizers
{

/**
 * Converts documents into streams of characters. This is the simplest
 * tokenizer.
 */
class character_tokenizer
    : public util::clonable<token_stream, character_tokenizer>
{
  public:
    /**
     * Creates a character_tokenizer.
     */
    character_tokenizer();

    /**
     * Sets the content for the tokenizer.
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * @return the next token in the document. This token will contain a
     * single character.
     */
    std::string next() override;

    /**
     * Determines if there are more tokens in the document.
     */
    operator bool() const override;

    /// Identifier for this tokenizer.
    const static util::string_view id;

  private:
    /// The buffered string content for this tokenizer
    std::string content_;

    /// Character index into the current buffer
    uint64_t idx_;
};
}
}
}
#endif
