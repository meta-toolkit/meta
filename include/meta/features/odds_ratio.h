/**
 * @file odds_ratio.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_ODDS_RATIO_H_
#define META_ODDS_RATIO_H_

#include "meta/config.h"
#include "meta/features/feature_selector.h"

namespace meta
{
namespace features
{
/**
 * Performs odds ratio feature selection:
 * \f$ OR(t, c_i) =
 * \log \frac{P(t|c_i)(1-P(t|\overline{c_i}))}{(1-P(t|c_i))P(t|\overline{c_i})}
 * \f$
 */
class odds_ratio : public feature_selector
{
  public:
    /// Inherit constructor.
    using feature_selector::feature_selector;

    /// Identifier for this feature_selector.
    const static std::string id;

    /**
     * Scores the (label, term) pair according to this feature selection
     * metric.
     * @param lbl
     * @param tid
     */
    virtual double score(const class_label& lbl, term_id tid) const override;
};
}
}
#endif
