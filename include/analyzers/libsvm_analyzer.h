/**
 * @file libsvm_analyzer.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_LIBSVM_ANALYZER_
#define META_LIBSVM_ANALYZER_

#include "analyzers/analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * libsvm_analyzer tokenizes documents that have been created from a
 * line_corpus, where each line is in libsvm input format and stored in the
 * document's content field.
 */
class libsvm_analyzer : public util::clonable<analyzer, libsvm_analyzer>
{
  public:
    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    virtual void tokenize(corpus::document& doc) override;

    /// Identifier for this analyzer.
    const static std::string id;
};
}
}

#endif
