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

#include "features/feature_selector.h"

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

    /**
     * This feature_selector is a friend of the factory method used to create it
     */
    template <class Selector, class ForwardIndex, class... Args>
    friend std::shared_ptr<Selector>
        make_selector(const std::string&, ForwardIndex, Args&&...);

    /**
     * Scores the (label_id, term) pair according to this feature selection
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

        double numerator = p_tc * p_ntnc - p_ntc * p_tnc;
        double denominator = p_c * (1.0 - p_c) * p_t * (1.0 - p_t);

        return (numerator * numerator) / denominator;
    }
};
}
}
#endif
