/**
 * @file information_gain.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INFORMATION_GAIN_H_
#define META_INFORMATION_GAIN_H_

#include "meta/config.h"
#include "meta/features/feature_selector.h"

namespace meta
{
namespace features
{
/**
 * Performs information gain feature selection:
 * \f$ IG(t, c_i) =
 * \sum_{c\in\{c_i, \overline{c_i}\}} \sum_{t'\in\{t,t'\}}P(t',c) \log
 * \frac{P(t',c)}{P(t')P(c)} \f$
 */
class information_gain : public feature_selector
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
