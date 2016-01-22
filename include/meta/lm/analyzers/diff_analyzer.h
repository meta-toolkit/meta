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
#include "meta/lm/diff.h"
#include "meta/analyzers/analyzer_factory.h"
#include "meta/analyzers/analyzer.h"
#include "meta/util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * Analyzes documents using lm::diff edits; see lm::diff for config file
 * information and further explanation.
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

    /// Identifier for this analyzer.
    const static util::string_view id;

  private:
    virtual void tokenize(const corpus::document& doc,
                          featurizer& counts) override;

    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;

    std::shared_ptr<lm::diff> diff_;
};

/**
 * Specialization of the factory method for creating diff analyzers.
 */
template <>
std::unique_ptr<analyzer> make_analyzer<diff_analyzer>(const cpptoml::table&,
                                                       const cpptoml::table&);
}
}
#endif
