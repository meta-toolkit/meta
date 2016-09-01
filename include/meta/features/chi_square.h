/**
 * @file chi_square.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CHI_SQUARE_H_
#define META_CHI_SQUARE_H_

#include "meta/config.h"
#include "meta/features/feature_selector.h"

namespace meta
{
namespace features
{
/**
 * Performs Chi square feature selection:
 * \f$ \chi^2(t, c_i) =
 * \frac{(P(t,c_i) P(\overline{t}, \overline{c_i}) - P(t, \overline{c_i})
 * P(\overline{t},c_i))^2}
 *  {P(t) P(\overline{t}) P(c_i) P(\overline{c_i})} \f$
 */
class chi_square : public feature_selector
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
