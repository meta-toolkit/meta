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
template <class T>
class diff_analyzer : public util::clonable<analyzer<T>, diff_analyzer<T>>
{
  public:
    using feature_map = typename diff_analyzer::feature_map;

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
    /**
     * Tokenizes a file into a document.
     * @param doc The document to store the tokenized information in
     */
    virtual void tokenize(const corpus::document& doc,
                          feature_map& counts) override;

    /// The token stream to be used for extracting tokens
    std::unique_ptr<token_stream> stream_;

    std::shared_ptr<lm::diff> diff_;
};

/**
 * Specialization of the traits class used by the factory method for
 * creating diff analyzers.
 */
template <class T>
struct analyzer_traits<diff_analyzer<T>>
{
    static std::unique_ptr<analyzer<T>> create(const cpptoml::table&,
                                               const cpptoml::table&);
};

// declare the valid instantiations for this analyzer
extern template class diff_analyzer<uint64_t>;
extern template class diff_analyzer<double>;

// declare the valid instantiations for this analyzer's trait class
extern template struct analyzer_traits<diff_analyzer<uint64_t>>;
extern template struct analyzer_traits<diff_analyzer<double>>;
}
}
#endif
