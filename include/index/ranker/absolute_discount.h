/**
 * @file absolute_discount.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef META_ABSOLUTE_DISCOUNT_H_
#define META_ABSOLUTE_DISCOUNT_H_

#include "index/ranker/lm_ranker.h"
#include "index/ranker/ranker_factory.h"

namespace meta
{
namespace index
{

/**
 * Implements the absolute discounting smoothing method.
 */
class absolute_discount : public language_model_ranker
{
  public:
    /**
     * The identifier of this ranker.
     */
    const static std::string id;

    /**
     * @param delta
     */
    absolute_discount(double delta = 0.7);

    /**
     * Calculates the smoothed probability of a term.
     * @param sd score_data for the current query
     */
    double smoothed_prob(const score_data& sd) const override;

    /**
     * A document-dependent constant.
     * @param sd score_data for the current query
     */
    double doc_constant(const score_data& sd) const override;

  private:
    /// the absolute discounting parameter
    const double delta_;
};

/**
 * Specialization of the factory method used to create absolute_discount
 * rankers.
 */
template <>
std::unique_ptr<ranker> make_ranker<absolute_discount>(const cpptoml::table&);
}
}
#endif
