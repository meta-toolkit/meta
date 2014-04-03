/**
 * @file ngram_simple_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_NGRAM_SIMPLE_ANALYZER_H_
#define META_NGRAM_SIMPLE_ANALYZER_H_

#include "analyzers/ngram/ngram_analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Derived classes from this simple ngram analyzer differ only in file
 * extensions and parsers used. Note this class is still abstract because
 * tokenize() is not defined.
 */
class ngram_simple_analyzer : public ngram_analyzer
{
  public:
    /**
     * Constructor.
     * @param n The value of n in ngram.
     */
    ngram_simple_analyzer(uint16_t n);

  protected:
    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     * @param parser The parser to use for this document
     */
    void simple_tokenize(io::parser& parser, corpus::document& doc);
};
}
}

#endif
