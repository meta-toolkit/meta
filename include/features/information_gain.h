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

#include "features/feature_selector.h"

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

    /**
     * This feature_selector is a friend of the factory method used to create it
     */
    template <class Selector, class ForwardIndex, class... Args>
    friend std::shared_ptr<Selector>
        make_selector(const std::string&, ForwardIndex, Args&&...);

    /**
     * Scores the (label, term) pair according to this feature selection
     * metric.
     * @param lid
     * @param tid
     */
    virtual double score(label_id lid, term_id tid) const override
    {
        double p_tc = term_and_class(tid, lid);
        double p_ntnc = not_term_and_not_class(tid, lid);
        double p_ntc = not_term_and_class(tid, lid);
        double p_tnc = term_and_not_class(tid, lid);
        double p_c = prob_class(lid);
        double p_t = prob_term(tid);
        double p_nc = 1.0 - p_c;
        double p_nt = 1.0 - p_t;

        double gain_tc = p_tc * std::log(p_tc / (p_t * p_c));
        double gain_ntnc = p_ntnc * std::log(p_ntnc / (p_nt * p_nc));
        double gain_ntc = p_ntc * std::log(p_ntc / (p_nt * p_c));
        double gain_tnc = p_tnc * std::log(p_tnc / (p_t * p_nc));

        // if any denominators were zero, make the expression zero
        if (std::isnan(gain_tc))
            gain_tc = 0.0;
        if (std::isnan(gain_ntnc))
            gain_ntnc = 0.0;
        if (std::isnan(gain_ntc))
            gain_ntc = 0.0;
        if (std::isnan(gain_tnc))
            gain_tnc = 0.0;

        return gain_tc + gain_ntnc + gain_ntc + gain_tnc;
    }
};
}
}
#endif
