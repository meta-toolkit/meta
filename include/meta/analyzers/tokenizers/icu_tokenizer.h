/**
 * @file icu_tokenizer.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_ICU_TOKENIZER_H_
#define META_ICU_TOKENIZER_H_

#include "meta/analyzers/filter_factory.h"
#include "meta/analyzers/token_stream.h"
#include "meta/utf/segmenter.h"
#include "meta/util/clonable.h"
#include "meta/util/pimpl.h"

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
 * Converts documents into streams of tokens by following the unicode
 * standards for sentence and word segmentation.
 *
 * Required config parameters: none.
 *
 * Optional config parameters:
 *
 * ~~~ toml
 * # lowercase two-letter or three-letter ISO-639 code
 * language = "en"
 *
 * # uppercase two-letter ISO-3116 code. If specified, the config must also
 * # specify the language.
 * country = "US"
 *
 * # whether to suppress the generation of "<s>" or "</s>"; useful for
 * # information retrieval with unigrams. Default is false.
 * suppress-tags = true
 * ~~~
 */
class icu_tokenizer : public util::clonable<token_stream, icu_tokenizer>
{
  public:
    /**
     * Creates an icu_tokenizer.
     * @param suppress_tags Whether to suppress "<s>" and "</s"> generation
     */
    explicit icu_tokenizer(bool suppress_tags = false);

    /**
     * Creates an icu_tokenizer with a specific segmenter.
     * @param segmenter The segmenter to use.
     * @param suppress_tags Whether to suppress "<s>" and "</s>" generation
     */
    explicit icu_tokenizer(utf::segmenter segmenter,
                           bool suppress_tags = false);

    /**
     * Copies an icu_tokenizer.
     * @param other The other icu_tokenizer to copy into this one
     */
    icu_tokenizer(const icu_tokenizer& other);

    /**
     * Moves an icu_tokenizer.
     * @param other The other icu_tokenizer to move into this one
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
     * @param content The string content to set
     */
    void set_content(std::string&& content) override;

    /**
     * @return the next token in the document. This will either by a
     * sentence boundary ("<s>" or "</s>"), a token consisting of
     * non-whitespace characters, or a token consisting of only
     * whitespace characters.
     */
    std::string next() override;

    /**
     * Determines if there are more tokens in the document.
     */
    operator bool() const override;

    /// Identifier for this tokenizer
    const static util::string_view id;

  private:
    /// Forward declaration of the impl
    class impl;

    /// The implementation for this tokenizer
    util::pimpl<impl> impl_;
};

/**
 * Specialization of the factory method use to create icu_tokenizers.
 */
template <>
std::unique_ptr<token_stream>
    make_tokenizer<icu_tokenizer>(const cpptoml::table& config);
}
}
}
#endif
