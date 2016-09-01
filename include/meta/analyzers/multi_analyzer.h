/**
 * @file multi_analyzer.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_MULTI_ANALYZER_
#define META_MULTI_ANALYZER_

#include <memory>
#include <vector>

#include "meta/analyzers/analyzer.h"
#include "meta/config.h"
#include "meta/util/clonable.h"

namespace meta
{
namespace analyzers
{

/**
 * The multi_analyzer class contains more than one analyzer. This is useful
 * for trying combined feature methods.
 *
 * For example, you could tokenize based on ngrams of words and parse tree
 * rewrite rules. The multi_analyzer keeps track of all the features in one set
 * for however many internal analyzers it contains.
 */
class multi_analyzer : public util::clonable<analyzer, multi_analyzer>
{
  public:
    /**
     * Constructs a multi_analyzer from a vector of other analyzers.
     * @param toks A vector of analyzers to combine features from
     */
    multi_analyzer(std::vector<std::unique_ptr<analyzer>>&& toks);

    /**
     * Copy constructor.
     * @param other The other multi_analyzer to copy from
     */
    multi_analyzer(const multi_analyzer& other);

  private:
    virtual void tokenize(const corpus::document& doc,
                          featurizer& counts) override;

  private:
    /// Holds all the analyzers in this multi_analyzer
    std::vector<std::unique_ptr<analyzer>> analyzers_;
};
}
}
#endif
