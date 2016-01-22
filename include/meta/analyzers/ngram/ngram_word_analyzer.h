/**
 * @file ngram_word_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_WORD_ANALYZER_H_
#define META_NGRAM_WORD_ANALYZER_H_

#include "meta/analyzers/analyzer_factory.h"
#include "meta/analyzers/ngram/ngram_analyzer.h"
#include "meta/util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents using their tokenized words.
 *
 * Required config parameters:
 * ~~~toml
 * [[analyzers]]
 * method = "ngram-word" # this analyzer
 * ngram = 1 # integer required
 * filter = "default-chain" # filter type required
 * ~~~
 *
 * Optional config parameters: none.
 *
 * @see https://meta-toolkit.org/analyzers-filters-tutorial.html
 */
class ngram_word_analyzer
    : public util::multilevel_clonable<analyzer, ngram_analyzer,
                                       ngram_word_analyzer>
{
    using base = util::multilevel_clonable<analyzer, ngram_analyzer,
                                           ngram_word_analyzer>;

  public:
    /**
     * Constructor.
     * @param n The value of n to use for the ngrams.
     * @param stream The stream to read tokens from.
     */
    ngram_word_analyzer(uint16_t n, std::unique_ptr<token_stream> stream);

    /**
     * Copy constructor.
     * @param other The other ngram_word_analyzer to copy from
     */
    ngram_word_analyzer(const ngram_word_analyzer& other);

    /// Identifier for this analyzer.
    const static util::string_view id;

  private:
    virtual void tokenize(const corpus::document& doc,
                          featurizer& counts) override;

    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;
};

/**
 * Specialization of the factory method for creating ngram_word_analyzers.
 */
template <>
std::unique_ptr<analyzer>
make_analyzer<ngram_word_analyzer>(const cpptoml::table&,
                                   const cpptoml::table&);
}
}
#endif
