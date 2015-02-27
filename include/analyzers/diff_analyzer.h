/**
 * @file diff_analyzer.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_DIFF_ANALYZER_H_
#define META_DIFF_ANALYZER_H_

#include "cpptoml.h"
#include "lm/diff.h"
#include "analyzers/analyzer_factory.h"
#include "analyzers/analyzer.h"
#include "util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents using their tokenized words.
 */
class diff_analyzer : public util::clonable<analyzer, diff_analyzer>
{
  public:
    diff_analyzer(const cpptoml::table& config,
            std::unique_ptr<token_stream> stream);

    /**
     * Copy constructor.
     * @param other The other diff_analyzer to copy from
     */
    diff_analyzer(const diff_analyzer& other);

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

    lm::diff diff_;
};

/**
 * Specialization of the factory method for creating diff_analyzers.
 */
template <>
std::unique_ptr<analyzer> make_analyzer<diff_analyzer>(
        const cpptoml::table&,
        const cpptoml::table&);
}
}
#endif
