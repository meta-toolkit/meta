/**
 * @file ngram_word_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_WORD_ANALYZER_H_
#define META_NGRAM_WORD_ANALYZER_H_

#include "analyzers/analyzer_factory.h"
#include "analyzers/ngram/ngram_analyzer.h"
#include "util/clonable.h"

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
template <class T>
class ngram_word_analyzer
    : public util::multilevel_clonable<analyzer<T>, ngram_analyzer<T>,
                                       ngram_word_analyzer<T>>
{
    using base = util::multilevel_clonable<analyzer<T>, ngram_analyzer<T>,
                                           ngram_word_analyzer>;

  public:
    using feature_map = typename ngram_word_analyzer::feature_map;

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
    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    virtual void tokenize(const corpus::document& doc,
                          feature_map& counts) override;

    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;
};

/**
 * Specialization of the traits class used by the factory method for
 * creating ngram_word_analyzers.
 */
template <class T>
struct analyzer_traits<ngram_word_analyzer<T>>
{
    static std::unique_ptr<analyzer<T>> create(const cpptoml::table&,
                                               const cpptoml::table&);
};

// declare the valid instantiations for this analyzer
extern template class ngram_word_analyzer<uint64_t>;
extern template class ngram_word_analyzer<double>;

// declare the valid instantiations for this analyzer's trait class
extern template struct analyzer_traits<ngram_word_analyzer<uint64_t>>;
extern template struct analyzer_traits<ngram_word_analyzer<double>>;
}
}
#endif
