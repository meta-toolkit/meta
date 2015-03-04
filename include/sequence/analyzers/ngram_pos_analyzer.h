/**
 * @file ngram_pos_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_POS_ANALYZER_H_
#define META_NGRAM_POS_ANALYZER_H_

#include <string>
#include "analyzers/analyzer_factory.h"
#include "sequence/sequence_analyzer.h"
#include "analyzers/ngram/ngram_analyzer.h"
#include "sequence/crf/crf.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents based on part-of-speech tags instead of words.
 * The recommended tokenizer for use with this analyzer is icu-tokenizer with no
 * other filters added. This tokenizer should be used to ensure that capital
 * letters and such may be used as features. Function words and stop words
 * should *not* be removed and words should not be stemmed for the same reason.
 */
class ngram_pos_analyzer
    : public util::multilevel_clonable<analyzer, ngram_analyzer,
                                       ngram_pos_analyzer>
{
    using base = util::multilevel_clonable<analyzer, ngram_analyzer,
                                           ngram_pos_analyzer>;

  public:
    /**
     * Constructor.
     * @param n The value of n to use for the ngrams.
     * @param stream The stream to read tokens from.
     * @param crf_prefix
     */
    ngram_pos_analyzer(uint16_t n, std::unique_ptr<token_stream> stream,
                       const std::string& crf_prefix);

    /**
     * Copy constructor.
     * @param other The other ngram_pos_analyzer to copy from
     */
    ngram_pos_analyzer(const ngram_pos_analyzer& other);

    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    virtual void tokenize(corpus::document& doc) override;

    /// Identifier for this analyzer.
    const static std::string id;

  private:
    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;

    /// The CRF used to tag the sentences
    std::shared_ptr<sequence::crf> crf_;

    /// Generates features for the CRF; const indicates testing mode
    const sequence::sequence_analyzer seq_analyzer_;
};

/**
 * Specialization of the factory method for creating ngram_pos_analyzers.
 */
template <>
std::unique_ptr<analyzer>
    make_analyzer<ngram_pos_analyzer>(const cpptoml::table&,
                                      const cpptoml::table&);
}

namespace sequence
{
/**
 * Registers analyzers provided by the meta-sequence-analyzers library.
 */
void register_analyzers();
}
}

#endif
