/**
 * @file ngram_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_ANALYZER_H_
#define META_NGRAM_ANALYZER_H_

#include <deque>

#include "meta/analyzers/analyzer.h"
#include "meta/util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents based on an ngram word model, where the value for n is
 * supplied by the user.  This class is abstract, as it only provides the
 * framework for ngram tokenization.
 */
class ngram_analyzer : public analyzer
{
  public:
    /**
     * Constructor.
     * @param n The value of n in ngram.
     */
    ngram_analyzer(uint16_t n);

    /**
     * @return the value of n used for the ngrams
     */
    virtual uint16_t n_value() const;

  protected:
    /**
     * Turns a list of words into an ngram string.
     * @param words The deque representing a list of words
     * @return the ngrams in string format
     */
    virtual std::string wordify(const std::deque<std::string>& words) const;

  private:
    /// The value of n for this ngram analyzer
    uint16_t n_val_;
};
}
}
#endif
